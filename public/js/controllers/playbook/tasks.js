define([
	'app',
	'jquery',
	'socketio',
	'services/tasks',
	'factories/task'
], function(app, $, io) {
	var socket = io();

	app.registerController('PlaybookTasksCtrl', ['$scope', 'tasks', 'Task', function($scope, tasks, Task) {
		$scope.tasks = tasks;

		tasks.get($scope.playbook, function () {
		});

		$scope.onPlaybookUpdate = function (data) {
			if (data.playbook_id != $scope.playbook.data._id) return;

			var task = $scope.findTask(data.task_id);

			if (!task) {
				// add task??
				$scope.tasks.tasks.splice(0, 0, new Task());
				$scope.tasks.tasks[0].data = data.task;
			} else {
				task.data = data.task;
			}

			if (!$scope.$$phase) {
				$scope.$digest();
			}
		};

		$scope.onTaskStatusUpdate = function (data) {
			if (data.playbook_id != $scope.playbook.data._id) {
				return;
			};

			var task = $scope.findTask(data.task_id);

			if (!task) {
				// do something??
			} else {
				$scope.$apply(function() {
            		task.data.status = data.task_status; 
            		//console.log('message:' + data.task_status);
          		});
			}
		};

		$scope.onSocketOutput = function (data) {
			if (data.playbook_id != $scope.playbook.data._id) return;

			var task = $scope.findTask(data.task_id);
			if (!task) return;

			if (!task.data.output) {
				task.data.output = "";
			}

			task.data.output += data.output;

			if (!$scope.$$phase) {
				$scope.$digest();
			}
		}

		$scope.onSocketZmqOutput = function (data) {
			if (data.playbook_id != $scope.playbook.data._id) return;

			var task = $scope.findTask(data.task_id);
			if (!task) return;

			if (!task.data.outputZmq) {
				task.data.outputZmq = "";
			}

			task.data.outputZmq += data.outputZmq;

			if (!$scope.$$phase) {
				$scope.$digest();
			}
		}

		$scope.findTask = function (task_id) {
			for (var i = 0; i < $scope.tasks.tasks.length; i++) {
				var task = $scope.tasks.tasks[i];

				if (task.data._id == task_id) {
					return task;
				}
			}

			return null;
		}

		$scope.removeTask = function (task) {
			console.log(JSON.stringify(task));
			task.delete($scope.playbook, task.data.job)
			.success(function () {
				console.log('dopo');
				$scope.status = 'Task Deleted';

				for (var i = 0; i < $scope.tasks.tasks.length; i++) {
					console.log('nel for');
					if ($scope.tasks.tasks[i] == task) {
						console.log('nell if del for');
						$scope.tasks.tasks.splice(i, 1);
						break;
					}
				}
			})
			.error(function (data, status) {
				$scope.status = 'Task Failed to Delete ('+status+'): '+data;
			});
		}

		$scope.openOutput = function (task) {
			$scope.openTask = task;
			$('#taskOutput').modal('show');
			$('#taskOutput').on('hide.bs.modal', $scope.closeOutput);
		}

		$scope.closeOutput = function () {
			$scope.openTask = null;

			$('#taskOutput').off('hide.bs.modal', $scope.closeOutput);
			$('#taskOutput').modal('hide');
		}

		$scope.openOutputZmq = function (task) {
			$scope.openTask = task;
			$('#taskOutputZmq').modal('show');
			$('#taskOutputZmq').on('hide.bs.modal', $scope.closeOutputZmq);
		}

		$scope.closeOutputZmq = function () {
			$scope.openTask = null;

			$('#taskOutputZmq').off('hide.bs.modal', $scope.closeOutputZmq);
			$('#taskOutputZmq').modal('hide');
		}

		socket.on('playbook.update', $scope.onPlaybookUpdate);
		socket.on('playbook.output', $scope.onSocketOutput);
		socket.on('task.statusUpdate', $scope.onTaskStatusUpdate);
		socket.on('zmqClient.output', $scope.onSocketZmqOutput);

		$scope.$on('$destroy', function () {
			// prevents memory leaks..
			socket.removeListener('playbook.update', $scope.onPlaybookUpdate);
			socket.removeListener('playbook.output', $scope.onSocketOutput);
			socket.removeListener('task.statusUpdate', $scope.onTaskStatusUpdate);
			socket.removeListener('zmqClient.output', $scope.onSocketZmqOutput);
		});
	}]);
});