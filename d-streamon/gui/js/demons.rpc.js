var server = {};

function Request(function_name, opt_argv) {
	if (!opt_argv)
		opt_argv = new Array();

	var callback = null;
	var len = opt_argv.length;
	if (len > 0 && typeof opt_argv[len - 1] == 'function') {
		callback = opt_argv[len - 1];
		opt_argv.length--;
	}

	var params = new Array();
	for (var i = 0; i < opt_argv.length; i++) {
		params.push(opt_argv[i]);
	}

	var data = {
		"method" : function_name,
		"params" : params
	};

	$.post("rpc.php", data, callback, "json");
}

function InstallFunction(obj, name) {
	obj[name] = function() {
		Request(name, arguments);
	};
}

InstallFunction(server, 'get_blocks_list');
InstallFunction(server, 'get_blocks_info');
