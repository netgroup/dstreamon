var async = require('async'),
	fs = require('fs'),
	spawn = require('child_process').spawn;

var config = require('./config'),
	models = require('./models'),
	app = require('./app');

var home = process.env.HOME + '/';
var user = process.env.USER;

var CronJob = require('cron').CronJob;

var m_hashtable = require('./m_hashtable');

//var c_hashtable = require('./c_hashtable');

exports.queue = async.queue(worker, 1);

function worker (task, callback) {
	// Task is to be model Task

	// Download the git project
	// Set up hosts file
	// Set up vault pwd file
	// Set up private key
	// Compile streamon
	// Execute ansible-playbook -i hosts --ask-vault-pass --private-key=~/.ssh/ansible_key task.yml
	// Create check monitoring
	// Start the zmq client

	async.waterfall([
		function (done) {

			console.log(JSON.stringify(task));

			task.populate('job', function (err) {
				done(err, task);
			});
		},
		function (task, done) {
			models.Playbook.findOne({ _id: task.playbook }, function (err, playbook) {
				done(err, task, playbook)
			});
		},
		function (task, playbook, done) {
			// mark task as running and send an update via socketio
			task.installation = 'Running';
			task.status = 'Checking...';

			app.io.emit('playbook.update', {
				task_id: task._id,
				playbook_id: playbook._id,
				task: task
			});

			models.Task.update({
				_id: task._id
			}, {
				$set: {
					installation: 'Running'
				}
			}, function (err) {
				done(err, task, playbook);
			});
		},
		function (task, playbook, done) {
			playbook.populate('identity', function (err) {
				done(err, task, playbook)
			});
		},
		installHostKeys,
		pullGit,
		setupHosts,
		setupVault,
		compileLib,
		playTheBook,
		checkStatus,
		zmqClient
	], function (err) {
		if (err) {
			console.error(err);
			task.installation = 'Failed';
		} else {
			task.installation = 'Completed';
		}

		var rmrf = spawn('rm', ['-rf', home + 'playbook_'+task.playbook])
		rmrf.on('close', function () {
			app.io.emit('playbook.update', {
				task_id: task._id,
				playbook_id: task.playbook,
				task: task
			});
			task.save();

			callback(err);
		});
	});
}

function installHostKeys (task, playbook, done) {
	// Install the private key
	playbookOutputHandler.call(task, "Updating SSH Keys\n");

	var location = home + '.ssh/id_rsa';
	fs.mkdir( home + '.ssh', 448, function() {
		async.parallel([
			function (done) {
				fs.writeFile(location, playbook.identity.private_key, {
					mode: 384 // base 8 = 0600
				}, done);
			},
			function (done) {
				fs.writeFile(location+'.pub', playbook.identity.public_key, {
					mode: 420 // base 8 = 0644
				}, done);
			},
			function (done) {
				var config = "Host *\n\
  StrictHostKeyChecking no\n\
  CheckHostIp no\n\
  PasswordAuthentication no\n\
  PreferredAuthentications publickey\n";

				fs.writeFile(home + '.ssh/config', config, {
					mode: 420 // 0644
				}, done);
			}
		], function (err) {
			playbookOutputHandler.call(task, "SSH Keys Updated.\n");
			done(err, task, playbook)
		});
	});
}

function pullGit (task, playbook, done) {
	// Pull from git
	playbookOutputHandler.call(task, "\nDownloading Playbook.\n");

	var install = spawn(config.path+"/scripts/pullGit.sh", [playbook.location, 'playbook_'+playbook._id], {
		cwd: home,
		env: {
			HOME: home,
			OLDPWD: home,
			PWD: home,
			LOGNAME: user,
			USER: user,
			TERM: 'xterm',
			SHELL: '/bin/bash',
			PATH: process.env.PATH+':/usr/local/sbin:/usr/local/bin:/sbin:/bin:/usr/sbin:/usr/bin:/root/bin',
			LANG: 'en_GB.UTF-8'
		}
	});
	install.stdout.on('data', playbookOutputHandler.bind(task));
	install.stderr.on('data', playbookOutputHandler.bind(task));

	install.on('close', function(code) {
		playbookOutputHandler.call(task, "\n\nPlaybook Downloaded.\n");

		done(null, task, playbook);
	});
}

function setupHosts (task, playbook, done) {
	var hostfile = '';

	models.HostGroup.find({
		playbook: playbook._id
	}, function (err, hostgroups) {
		async.each(hostgroups, function (group, cb) {
			models.Host.find({
				group: group._id
			}, function (err, hosts) {
				hostfile += "["+group.name+"]\n";

				for (var i = 0; i < hosts.length; i++) {
					hostfile += hosts[i].hostname;
					if ( hosts[i].vars && hosts[i].vars.length > 0 ) {
						hostfile +=" " + hosts[i].vars;
					}

					hostfile += "\n";
				}

				cb();
			});
		}, function () {
			playbookOutputHandler.call(task, "\nSet up Ansible Hosts file with contents:\n"+hostfile+"\n");

			fs.writeFile(home + 'playbook_'+playbook._id+'/semaphore_hosts', hostfile, function (err) {
				done(err, task, playbook);
			});
		});
	});
}

function setupVault (task, playbook, done) {
	fs.writeFile(home + 'playbook_'+playbook._id+'/semaphore_vault_pwd', playbook.vault_password, function (err) {
		done(err, task, playbook);
	})
}

function compileLib (task, playbook, done){
	console.log ("Compile");
	//console.log (home);
	
	var compile = spawn('./start.sh', ['config_portscan.xml'], {
		cwd: '/opt/semaphore/d-streamon'
	});

	compile.stdout.on('data', playbookOutputHandler.bind(task));
	compile.stderr.on('data', playbookOutputHandler.bind(task));

	compile.on('close', function(code) {
		
		if (code !== 0) {
			// Task failed
			return done('Compile error');
		}
		else {
			console.log('Compile ok!');	
		}

	});

	done(null, task, playbook);

	//done(err, task, playbook);
}

function playTheBook (task, playbook, done) {
	playbookOutputHandler.call(task, "\nStarting play "+task.job.play_file+".\n");

	var args = ['-v', '-i', 'semaphore_hosts'];
	if (task.job.use_vault && playbook.vault_password && playbook.vault_password.length > 0) {
		args.push('--vault-password-file='+'semaphore_vault_pwd');
	}

	// private key to login to server[s]
	args.push('--private-key=' + home + '.ssh/id_rsa');

	// the playbook file
	args.push(task.job.play_file);

	// the extra vars
	if( task.job.extra_vars != undefined && task.job.extra_vars.length>0){
		var vars = task.job.extra_vars.split(" ");

		for (var i=0; i < vars.length; i++){
			args.push('-e');
			args.push(vars[i]);
		}
	}

	console.log(args);

	var playbookgo = spawn("ansible-playbook", args, {
		cwd: home + 'playbook_'+playbook._id,
		env: {
			HOME: home,
			OLDPWD: home,
			PWD: home + 'playbook_'+playbook._id,
			LOGNAME: user,
			USER: user,
			TERM: 'xterm',
			SHELL: '/bin/bash',
			PATH: process.env.PATH+':/usr/local/sbin:/usr/local/bin:/sbin:/bin:/usr/sbin:/usr/bin:/root/bin',
			LANG: 'en_GB.UTF-8',
			PYTHONPATH: process.env.PYTHONPATH,
			PYTHONUNBUFFERED: 1
		}
	});

	console.log(playbookgo);
	playbookgo.stdout.on('data', playbookOutputHandler.bind(task));
	playbookgo.stderr.on('data', playbookOutputHandler.bind(task));

	playbookgo.on('close', function(code) {
		console.log('done.', code);

		if (code !== 0) {
			// Task failed
			return done('Failed with code '+code);
		}

		done(null, task, playbook);
	});
}

function checkStatus (task, playbook, done){

	playbookOutputHandler.call(task, "\nStarting checkStatus.\n");

	var args = ['-v', '-i', 'semaphore_hosts'];
	if (task.job.use_vault && playbook.vault_password && playbook.vault_password.length > 0) {
		args.push('--vault-password-file='+'semaphore_vault_pwd');
	}

	// private key to login to server[s]
	args.push('--private-key=' + home + '.ssh/id_rsa');

	// the playbook file
	args.push('status.yml');

	// the extra vars
	if( task.job.extra_vars != undefined && task.job.extra_vars.length>0){
		var vars = task.job.extra_vars.split(" ");

		for (var i=0; i < vars.length; i++){
			args.push('-e');
			args.push(vars[i]);
		}
	}

	console.log(args);

	var opt = spawn('mkdir', ['-p', '/home/vagrant/status'], {
		cwd: '/'
	});

	opt.stdout.on('data', playbookOutputHandler.bind(task));
	opt.stderr.on('data', playbookOutputHandler.bind(task));


	opt = spawn('mkdir', ['-p', 'playbook_'+playbook._id], {
		cwd: '/home/vagrant/status'
	});

	opt.stdout.on('data', playbookOutputHandler.bind(task));
	opt.stderr.on('data', playbookOutputHandler.bind(task));

	opt.on('close', function(code) {
		
		if (code !== 0) {
			// Task failed
			return done('Create directory error');
		}
		else {
			console.log('Create directory ok!');	
		}
	});

	opt = spawn('cp', ['semaphore_hosts', home+'status/playbook_'+playbook._id+'/semaphore_hosts'], {
		cwd: home + 'playbook_'+playbook._id
	});

	opt.stdout.on('data', playbookOutputHandler.bind(task));
	opt.stderr.on('data', playbookOutputHandler.bind(task));

	opt.on('close', function(code) {
		
		if (code !== 0) {
			// Task failed
			return done('Copy hostfile error');
		}
		else {
			console.log('Copy hostfile ok!');	
		}
	});

	opt = spawn('cp', ['status.yml', home+'status/playbook_'+playbook._id+'/status.yml'], {
		cwd: '/opt/semaphore/'
	});

	opt.stdout.on('data', playbookOutputHandler.bind(task));
	opt.stderr.on('data', playbookOutputHandler.bind(task));

	opt.on('close', function(code) {
		
		if (code !== 0) {
			// Task failed
			return done('Copy status.yml error');
		}
		else {
			console.log('Copy status.yml ok!');	
		}
	});

	var cronJob = new CronJob({
		cronTime: '*/5 * * * * *',
  		onTick: function() {
  			var last;
     		fs.stat(home+'status/playbook_'+playbook._id+'/resp', function(err, stat) {
    			if(err == null) {
        			//console.log('File exists');
					fs.readFile(home+'status/playbook_'+playbook._id+'/resp', 'utf8', function (err,data) {
  						if (err) {
    						return ('Failed on readFile '+err);
  						}
  						else {
  							last = data;
  						}
					});    			
    			}
    			else if(err.code == 'ENOENT') {
        			//TO DO per qualche motivo il file non c'è...
    			}
    			else {
    				return ('Some other error on stat: '+err.code);
    			}
			});

     		var p_playbookgo = spawn("ansible-playbook", args, {
				cwd: home+'status/playbook_'+playbook._id+'/',
				env: {
					HOME: home,
					OLDPWD: home,
					PWD: home+'status/playbook_'+playbook._id+'/',
					LOGNAME: user,
					USER: user,
					TERM: 'xterm',
					SHELL: '/bin/bash',
					PATH: process.env.PATH+':/usr/local/sbin:/usr/local/bin:/sbin:/bin:/usr/sbin:/usr/bin:/root/bin',
					LANG: 'en_GB.UTF-8',
					PYTHONPATH: process.env.PYTHONPATH,
					PYTHONUNBUFFERED: 1
				}
			});

			//p_playbookgo.stdout.on('data', playbookOutputHandler.bind(task));
			//p_playbookgo.stderr.on('data', playbookOutputHandler.bind(task));

			p_playbookgo.on('close', function(code) {
				if (code !== 0) {
					// Task failed
					fs.stat(home+'status/playbook_'+playbook._id+'/resp', function(err, stat) {
    					if(err == null) {
        					//console.log('File exists');
							fs.readFile(home+'status/playbook_'+playbook._id+'/resp', 'utf8', function (err,data) {
  								if (err) {
    								return ('Failed on readFile '+err);
  								}
  								else {
  									if (parseInt(data) != 42){
  										fs.writeFile(home+'status/playbook_'+playbook._id+'/resp', "42", function(err) {
    										if(err) {
        										return console.log(err);
    										}
    										console.log("The file 42 was saved!");
										}); 
										task.status = "Checking...";
    									app.io.emit('task.statusUpdate', {
											task_id: task._id,
											playbook_id: task.playbook,
											task_status: task.status
										});
  									}
  								}
							});    			
    					}
    					else if(err.code == 'ENOENT') {
        					//TO DO per qualche motivo il file non c'è...
    					}
    					else {
    						return ('Some other error on stat: '+err.code);
    					}
					});
					return ('Failed on playbook_go with code: '+code);
				}
				else{
					fs.stat(home+'status/playbook_'+playbook._id+'/resp', function(err, stat) {
    					if(err == null) {
        					//console.log('File exists');
						    fs.readFile(home+'status/playbook_'+playbook._id+'/resp', 'utf8', function (err,data) {
  								if (err) {
    								return ('Failed on readFile: '+err);
  								}
  								else {
  									if (last != data) {
  										if (data == 1){
  											task.status = "Running";
  										}
  										else if (data == 0 ){
  											task.status = "Stopped"
  										}
  										app.io.emit('task.statusUpdate', {
											task_id: task._id,
											playbook_id: task.playbook,
											task_status: task.status
										});
  									}
  								}
							});			
    					}
    					else if(err.code == 'ENOENT') {
        					//TO DO per qualche motivo il file non c'è...
    					}
    					else {
    						return ('Some other error on stat 2: '+ err.code);
    					}
					});
				}
			});
			return 0;
  		},
  		start: false
	});
	
	console.log(task._id);
	m_hashtable.put(task._id, cronJob);

	var playbookgo = spawn("ansible-playbook", args, {
		cwd: home+'status/playbook_'+playbook._id+'/',
		env: {
			HOME: home,
			OLDPWD: home,
			PWD: home+'status/playbook_'+playbook._id+'/',
			LOGNAME: user,
			USER: user,
			TERM: 'xterm',
			SHELL: '/bin/bash',
			PATH: process.env.PATH+':/usr/local/sbin:/usr/local/bin:/sbin:/bin:/usr/sbin:/usr/bin:/root/bin',
			LANG: 'en_GB.UTF-8',
			PYTHONPATH: process.env.PYTHONPATH,
			PYTHONUNBUFFERED: 1
		}
	});

	//console.log(playbook);

	//playbookgo.stdout.on('data', playbookOutputHandlerNoLog.bind(task));
	//playbookgo.stderr.on('data', playbookOutputHandlerNoLog.bind(task));

	playbookgo.on('close', function(code) {
		console.log('done.', code);

		if (code !== 0) {
			// Task failed
			spawn("echo", ['42', '>', 'resp'], {
				cwd: home+'status/playbook_'+playbook._id+'/',
				env: {
					HOME: home,
					OLDPWD: home,
					PWD: home+'status/playbook_'+playbook._id+'/',
					LOGNAME: user,
					USER: user,
					TERM: 'xterm',
					SHELL: '/bin/bash',
					PATH: process.env.PATH+':/usr/local/sbin:/usr/local/bin:/sbin:/bin:/usr/sbin:/usr/bin:/root/bin',
					LANG: 'en_GB.UTF-8',
					PYTHONPATH: process.env.PYTHONPATH,
					PYTHONUNBUFFERED: 1
				}
			});
			task.status = "Checking..."
    		app.io.emit('task.statusUpdate', {
				task_id: task._id,
				playbook_id: task.playbook,
				task_status: task.status
			});
			task.save();
			return done('Failed with code '+code);
		}
		else {
			fs.stat(home+'status/playbook_'+playbook._id+'/resp', function(err, stat) {
    			if(err == null) {
        			
        			console.log('File exists');
					
					fs.readFile(home+'status/playbook_'+playbook._id+'/resp', 'utf8', function (err,data) {
  						if (err) {
    						return done('Failed on readFile '+err);
  						}
  						else {
  							if (data == 1) {
  								task.status = "Running";
  							}
  							else if (data == 0 ) {
  								task.status = "Stopped"
  							}
  							task.save();
  							app.io.emit('task.statusUpdate', {
								task_id: this._id,
								playbook_id: this.playbook,
								task_status: task.status
							});
  							cronJob.start();
							done(null, task, playbook);
  						}
					});
					//done();    			
    			}
    			else if(err.code == 'ENOENT') {
        			//per qualche motivo il file non c'è...
    			}
    			else {
    				//console.log('Some other error: ', err.code);
    				spawn("echo", ['42', '>', 'resp'], {
						cwd: home+'status/playbook_'+playbook._id+'/',
						env: {
							HOME: home,
							OLDPWD: home,
							PWD: home+'status/playbook_'+playbook._id+'/',
							LOGNAME: user,
							USER: user,
							TERM: 'xterm',
							SHELL: '/bin/bash',
							PATH: process.env.PATH+':/usr/local/sbin:/usr/local/bin:/sbin:/bin:/usr/sbin:/usr/bin:/root/bin',
							LANG: 'en_GB.UTF-8',
							PYTHONPATH: process.env.PYTHONPATH,
							PYTHONUNBUFFERED: 1
						}
					});
    				task.status = "Checking..."
    				task.save();
    				app.io.emit('task.statusUpdate', {
						task_id: task._id,
						playbook_id: task.playbook,
						task_status: task.status
					});
    				return done('Some other error: ', err.code);
    			}
			});
		}
		//done();
	});
}

function zmqClient (task, playbook, done){

	//playbookOutputHandler.call(task, "\nStarting Zmq Client.\n");

	console.log("\nStarting Zmq Client.\n");

	models.HostGroup.find({
		playbook: playbook._id
	}, function (err, hostgroups) {
		async.each(hostgroups, function (group, cb) {
			models.Host.find({
				group: group._id
			}, function (err, hosts) {
				if (err){
					return ('Error in zmqClient: '+err);
				}

				for (var i = 0; i < hosts.length; i++) {
					//console.log (hosts[i].hostname);
					var current = hosts[i];


					var zmqClientgo = spawn("./zmq_client", ["tcp://"+hosts[i].hostname+":5560"], {
						cwd: '/opt/semaphore/d-streamon/zmq-client/',
						env: {
							HOME: home,
							OLDPWD: home,
							PWD: '/opt/semaphore/d-streamon/zmq-client/',
							LOGNAME: user,
							USER: user,
							TERM: 'xterm',
							SHELL: '/bin/bash',
							PATH: process.env.PATH+':/usr/local/sbin:/usr/local/bin:/sbin:/bin:/usr/sbin:/usr/bin:/root/bin',
							LANG: 'en_GB.UTF-8',
							PYTHONPATH: process.env.PYTHONPATH,
							PYTHONUNBUFFERED: 1
						}
					});

					fs.stat(home+'status/playbook_'+playbook._id+'/resp', function(err, stat) {
    					if(err == null) {
        					//console.log('File exists');
        					//read File
							fs.readFile('/opt/semaphore/d-streamon/zmq-client/zmq_client.pid', 'utf8', function (err,data) {
  								if (err) {
    								return ('Failed on readFile '+err);
  								}
  								else {
  									current.zmq_pid = data;
  									current.save();
  									//fs.unlink('/opt/semaphore/d-streamon/zmq-client/zmq_client.pid');
  								}
							});    			
    					}
    					else if(err.code == 'ENOENT') {
        					//TO DO per qualche motivo il file non c'è...
    					}
	    				else {
    						return ('Some other error on stat: '+err.code);
    					}
					});

					zmqClientgo.stdout.on('data', zmqClientOutputHandler.bind(task));
					zmqClientgo.stderr.on('data', zmqClientOutputHandler.bind(task));

					zmqClientgo.on('close', function(code) {
						//...do something?
					})
				}
				cb();
			});
		}, function (err) {
			if (err){
				return err;
			}
			console.log('Clients zmq start');
		});
	});
	done();
}


function playbookOutputHandler (chunk) {

	chunk = chunk.toString('utf8');

	if (!this.output) {
		this.output = "";
	}

	this.output += chunk;
	app.io.emit('playbook.output', {
		task_id: this._id,
		playbook_id: this.playbook,
		output: chunk
	});

	console.log(chunk);
}

function zmqClientOutputHandler (chunk) {

	chunk = chunk.toString('utf8');

	if (!this.output) {
		this.outputZmq = "";
	}

	this.outputZmq += chunk;
	app.io.emit('zmqClient.output', {
		task_id: this._id,
		playbook_id: this.playbook,
		outputZmq: chunk
	});

	console.log(chunk);
}
