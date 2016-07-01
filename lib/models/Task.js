var mongoose = require('mongoose')
var ObjectId = mongoose.Schema.ObjectId;

var schema = mongoose.Schema({
	created: {
		type: Date,
		default: Date.now
	},
	job: {
		type: ObjectId,
		ref: 'Job'
	},
	playbook: {
		type: ObjectId,
		ref: 'Playbook'
	},
	output: String,
	outputZmq: String,
	installation: {
		type: String,
		enum: ['Completed', 'Failed', 'Running', 'Queued', 'Uninstalling...']
	},
	status: {
		type: String,
		enum: ['Running', 'Checking...', 'Stopped']
	}
});

schema.index({
	status: 1
});

module.exports = mongoose.model('Task', schema);