var models = require('../../models')
var mongoose = require('mongoose')
var express = require('express')

var jobRunner = require('../../runner');
var m_hashtable = require('../../m_hashtable');

var app = require('../../app');

var spawn = require('child_process').spawn;

var home = process.env.HOME + '/';
var user = process.env.USER;

exports.unauthorized = function (app, template) {
	template([
		'view'
	], {
		prefix: 'task'
	});
}

exports.httpRouter = function (app) {
}

exports.router = function (app) {
	var task = express.Router();

	task.get('/', view)
		.delete('/', remove)

	app.param('task_id', get)
	app.use('/playbook/:playbook_id/job/:job_id/task/:task_id', task);
}

function get (req, res, next, id) {
	models.Task.findOne({
		_id: id
	}).exec(function (err, task) {
		if (err || !task) {
			return res.send(404);
		}

		req.task = task;
		next();
	});
}

function view (req, res) {
	res.send(req.task);
}

function remove (req, res) {
	if (req.task.installation == 'Running' || req.task.installation == 'Uninstalling...') {
		return res.send(400, 'Job is Running or Uninstalling.');
	}

	jobRunner.queue.pause();

	//mettere il task in uninstalling
	//update Mongo
	models.Task.update({
		_id: req.task._id
	}, {
		$set: {
			status: 'Uninstalling...'
		}
	}, function (err) {
			if (err){
				return console.log(err);	
			}
		}
	);
	//Update client
	app.io.emit('playbook.update', {
		task_id: req.task._id,
		playbook_id: req.task.playbook,
		task_status: 'Uninstalling...'
	});

	var semaphore_hosts = home + 'status/playbook_' + req.task.playbook + '/semaphore_hosts';

	console.log(semaphore_hosts); 

	//rimuovere la sonda nel host
	var playbook_uninstall = spawn("ansible-playbook", ['-v', '-i', semaphore_hosts, '--private-key='+ home + '.ssh/id_rsa', 'uninstall.yml'], {
		cwd: '/opt/semaphore/playbooks',
			env: {
				HOME: home,
				OLDPWD: home,
				PWD: '/opt/semaphore/playbooks',
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

	playbook_uninstall.stdout.on('data', playbookOutputHandler.bind(req.task));
	playbook_uninstall.stderr.on('data', playbookOutputHandler.bind(req.task));

	playbook_uninstall.on('close', function(code) {

		//rimuovere il monitor nell'hash table
		if (m_hashtable.remove(req.task._id)) {
			console.log('job removed from hashtable');
		}
		else {
			console.log('job NOT removed from hashtable');
		}

		//rimuovere il task da mongo e dal lato client
		for (var i = 0; i < jobRunner.queue.tasks.length; i++) {
			if (jobRunner.queue.tasks[i].data._id.toString() == req.task._id.toString()) {
				// This is our task
				jobRunner.queue.tasks.splice(i, 1);
				break;
			}
		}
		jobRunner.queue.resume();

		console.log(req.task._id);

		req.task.remove(function (err) {
			res.send(201);
		})
	});
}

function playbookOutputHandler (chunk) {

	chunk = chunk.toString('utf8');

	/*if (!this.output) {
		this.output = "";
	}

	this.output += chunk;
	app.io.emit('playbook.output', {
		task_id: this._id,
		playbook_id: this.playbook,
		output: chunk
	});*/

	console.log(chunk);
}