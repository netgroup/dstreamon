var mongoose = require('mongoose')
var ObjectId = mongoose.Schema.ObjectId;

var schema = mongoose.Schema({
	created: {
		type: Date,
		default: Date.now
	},
	/*job: {
		type: ObjectId,
		ref: 'Job'
	},
	playbook: {
		type: ObjectId,
		ref: 'Playbook'
	},
	task: {
		type: ObjectId,
		ref: 'Task'
	},*/
	job_id: String,
	playbook_id: String,
	task_id: String,
	output: String,
	command: String,
	status: {
		type: String,
		enum: ['Running', 'Checking...', 'Stopped']
	}
});

schema.index({
	status: 1
});

module.exports = mongoose.model('Periodic', schema);