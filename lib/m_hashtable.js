var models = require('./models');
var fs = require('fs');
var spawn = require('child_process').spawn;

var HashTable = require('hashtable');
var hashtable = new HashTable();

var CronJob = require('cron').CronJob;

var home = process.env.HOME + '/';
var user = process.env.USER;

var app = require('./app');

//Initialize hashtable
models.Task.find({}, function(err, resp) {
    resp.forEach(function(resp) {
    	console.log(resp._id);

    	fs.stat(home+'status/playbook_'+resp.playbook+'/resp', function(err, stat) {
    		if(err == null) {
				fs.unlink(home+'status/playbook_'+resp.playbook+'/resp');    			
    		}
    		else if(err.code == 'ENOENT') {
        		//ok!!!
    		}
	    	else {
    			return ('Some other error on stat: '+err.code);
    		}
		
			var cronJob = new CronJob({
				cronTime: '*/5 * * * * *',
  				onTick: function() {
  					var home = process.env.HOME + '/';
	  				var last;
	     			var p_playbookgo = spawn("ansible-playbook", ['-v', '-i', 'semaphore_hosts', '--private-key='+ home + '.ssh/id_rsa', 'status.yml'], {
						cwd: home+'status/playbook_'+resp.playbook+'/',
						env: {
							HOME: home,
							OLDPWD: home,
							PWD: home+'status/playbook_'+resp.playbook+'/',
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

					//p_playbookgo.stdout.on('data', playbookOutputHandler.bind(resp));
					//p_playbookgo.stderr.on('data', playbookOutputHandler.bind(resp));

					p_playbookgo.on('close', function(code) {
						if (code !== 0) {
							// Task failed and stat file
							fs.stat(home+'status/playbook_'+resp.playbook+'/resp', function(err, stat) {
	    						if(err == null) {
	        						//console.log('File exists');
	        						//Read File
									fs.readFile(home+'status/playbook_'+resp.playbook+'/resp', 'utf8', function (err,data) {
	  									if (err) {
	    									return ('Failed on readFile '+err);
	  									}
	  									else {
	  										if (parseInt(data) != 42){
	  											//Write file
	  											fs.writeFile(home+'status/playbook_'+resp.playbook+'/resp', "42", function(err) {
	    											if(err) {
	        											return console.log(err);
	    											}
	    											console.log("The file 42 was saved!");
												});
												//Update Mongo
												models.Task.update({
													_id: resp._id
												}, {
													$set: {
														status: 'Checking...'
													}
												}, function (err) {
													return console.log(err);
												});
											
												resp.status = "Checking...";
	    									
	    										//Update Client
	    										app.io.emit('task.statusUpdate', {
													task_id: resp._id,
													playbook_id: resp.playbook,
													task_status: resp.status
												});
	  										}
	  									}
									});    			
	    						}
		    					else if(err.code == 'ENOENT') {
	    	    					//TO DO per qualche motivo il file non c'è...devo recuperarlo da mongo
	    						}
	    						else {
	    							return ('Some other error on stat: '+err.code);
	    						}
							});
							return ('Failed on playbook_go with code: '+code);
						}
						else{
							//File stat
							fs.stat(home+'status/playbook_'+resp.playbook+'/resp', function(err, stat) {
	    						if(err == null) {
	        						//console.log('File exists');
								    //File read
								    fs.readFile(home+'status/playbook_'+resp.playbook+'/resp', 'utf8', function (err,data) {
	  									if (err) {
	    									return ('Failed on readFile: '+err);
	  									}
	  									else {
	  										if (last != data) {
	  											if (data == 1){
	  												resp.status = "Running";
	  											}
	  											else if (data == 0 ){
	  												resp.status = "Stopped"
		  										}
	  											//Update Mongo
	  											models.Task.update({
													_id: resp._id
												}, {
													$set: {
														status: resp.status
													}
												}, function (err) {
													if (err){
													return console.log(err);	
													}
												});
		  										//Update client
	  											app.io.emit('task.statusUpdate', {
													task_id: resp._id,
													playbook_id: resp.playbook,
													task_status: resp.status
												});
	  										}
	  									}
									});			
	    						}
	    						else if(err.code == 'ENOENT') {
	        						//TO DO per qualche motivo il file non c'è...devo recuperarlo da mongo
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

			hashtable.put(resp._id, cronJob);

			if (hashtable.has(resp._id)) {
				cronJob.start();
			}
			else {
				return ('Some error on hashtable.put');
			}
		}); 
    });
});

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

	//console.log(chunk);
}

exports.put = function (id, cronJob){
	//console.log(id);
	hashtable.put(id,cronJob);
	

	//console.log('inserito!!!');
}

exports.remove = function (id){
	//console.log('nell hash'+id);
	if (hashtable.has(id)) {
		//console.log('qualcosa cè');
		var cronJob = hashtable.get(id);
		cronJob.stop();
	}
	return (hashtable.remove(id));
}