var dragVPosition = 0;

var sourceEndpoint = {
	    isSource : true,
	    anchor : "RightMiddle",
        maxConnections : -1
    },
    targetEndpoint = {
        isTarget : true,
	    anchor : "LeftMiddle",
        maxConnections : -1
    };

function newId() {
	var today = new Date;
	return today.getTime();
}

function openGeneralDlg(g, t, d, f) {
	$(g + "-description")[0].innerText = d;
	var dlg = $(g); 
	dlg.dialog("option", "title", t);
	if (typeof(f) == "function") {
		dlg.dialog("option", "buttons", {
            "Ok" : function() {
            	$(this).dialog("close");
            	f();
        	},
        	"Cancel" : function() {
        		$(this).dialog("close");
        	}
		});
	}
	dlg.dialog("open");
}

function openProgressBar(t, d) {
	openGeneralDlg("#dialog-progress-bar", t, d);
}

function openMsgbox(t, d) {
	openGeneralDlg("#dialog-msgbox", t, d);
}

function openConfirm(t, d, yesfunction) {
	openGeneralDlg("#dialog-msgbox-yesno", t, d, yesfunction);
}

function addBlock(id, idComp, name, type, posTop, posLeft) {
	var idworkspace = "#" + idComp + "-workspace";
	objworkspace = $(idworkspace),
	blockcontent = $("#block-template")[0].innerHTML;
	objworkspace.append(blockcontent);
	
	var newblock = objworkspace.find("#block")[0];
	newblock.id = id;
	newblock.type = type;
	newblock = $("#" + id);
	newblock.css("position", "inherit");
	newblock.css("top", posTop - objworkspace.offset().top - 1);
	newblock.css("left", posLeft - objworkspace.offset().left - 1);
	newblock.dblclick(function() {
		openConfigDialog($(this));
	});
	jsPlumb.draggable(newblock, { containment: idworkspace });
	jsPlumb.addEndpoint(newblock, sourceEndpoint);
	jsPlumb.addEndpoint(newblock, targetEndpoint);
	
	var newblockname = newblock.find("#block-name")[0];
	newblockname.id = "block-name-" + id;
	newblockname.innerHTML = name;
}

function addTab(id, t) {
    $("#tabs").tabs("add", "#tabs-" + id, t);
	var newtab = $("#tabs-" + id);
	$.each(["drag-container","workspace","start","blocks-available","blocks-accordion","block-A","block-B","block-C"],
		function(index, value) {
			newtab.find("#" + value)[0].id = id + "-" + value;
		}
	);
	
	var startBox = [ $("#" + id + "-start") ];
	jsPlumb.draggable(startBox, { containment: "#" + id + "-workspace" });
	jsPlumb.addEndpoint(startBox, sourceEndpoint);
	
	// Available blocks
	$("#" + id + "-blocks-accordion").accordion({
		fillSpace: true,
		icons: {
			header: "ui-icon-circle-arrow-e",
			headerSelected: "ui-icon-circle-arrow-s"
		}
	});

	$("#" + id + "-blocks-available div").draggable({
		appendTo : "#" + id + "-drag-container",
		containment: "#" + id + "-drag-container",
		stack: "#" + id + "-blocks-available div",
//		revert : true,
		helper : "clone",
		start : function(event, ui) {
			dragVPosition = ui.position.top;
		},
		stop : function(event, ui) {
			if (ui.position.left < $("#" + id + "-workspace")[0].offsetWidth - ui.helper[0].clientWidth) {
				addBlock(newId(), id,
						ui.helper[0].innerText,
						ui.helper[0].attributes["type"].value,
						ui.offset.top, ui.offset.left);
			}
		}
	});
}

function closeProgressBar() {
	$("#dialog-progress-bar").dialog("close");
}

function getSavedCompositions() {
	openProgressBar("Opening", "Get list of saved compositions...");
	$.ajax({
		dataType : "jsonp",
		type : "POST",
		url : "getSavedCompositions.php",
		jsonp : "callback",
        jsonpCallback: "loadSavedCompositions"
	});
	
	return false;
}

function loadSavedCompositions(xml) {
	closeProgressBar();
	
	$("#composition-select")[0].options.length = 0;
	$(xml).find("composition").each(function() {
		var element = document.createElement("option");
		element.text = $(this).text();
		element.value = $(this).text();
		$("#composition-select")[0].add(element, null);
	});
    $("#dialog-open-composition").dialog("open");
}

function getComposition(id, t) {
	openProgressBar("Opening", "Opening composition " + t + "...");
	$.ajax({
		dataType : "jsonp",
		type : "POST",
		url : "getComposition.php",
		data : { "id" : id },
		jsonp : "callback",
        jsonpCallback: "loadComposition"
	});
	
	return false;
}

function loadComposition(xml) {
	closeProgressBar();
	
	addTab($(xml).attr("id"), $(xml).attr("id"));
	var objworkspace = $("#" + $(xml).attr("id") + "-workspace");
	$(xml).find("block").each(function (index, value) {
		addBlock(
			$(value).attr("id"),
			$(xml).attr("id"),
			$(value).attr("id"),
			$(value).attr("type"),
			objworkspace.offset().top + 1 + index * 50,
			objworkspace.offset().left + 1 + index * 50
		);
	});
	
	$(xml).find("connection").each(function (index, value) {
		jsPlumb.connect({
			source : $(value).attr("src_block"),
			target : $(value).attr("dst_block")
		});
	});
//	
//	  $(xml).find("Tutorial").each(function()
//			  {
//			    $("#output").append($(this).attr("author") + "<br />");
//			  });
//	
//	$.each(conns, function(index, value) {
//		jsPlumb.connect({
//			source : value.sourceId,
//			target : value.targetId
//		});
//	});
	
	return false;
}

function saveApplication() {
	var data = {};
	
	var dataapplication = {
		id : $("#tabs").find(".ui-tabs-panel")[($("#tabs").tabs("option", "selected"))].id.substr(5),
		name : $("#tabs").find(".ui-tabs-selected")[0].innerText
	};
	data.application = dataapplication;
	
	var datablocks = [];
	var blocks = $("#" + dataapplication.id + "-workspace .block");
	$.each(blocks, function(index, value) {
		datablocks.push({
			"id" : value.id,
			"name" : value.innerText,
			"type" : value.type,
			"top" : value.offsetTop,
			"left" : value.offsetLeft
		});
	});
	data.blocks = datablocks;
	
	var dataconns = [];
	var conns = jsPlumb.getConnections();
	$.each(conns, function(index, value) {
		for (i = 0; i < blocks.length; ++i) {
			if ((blocks[i].id == value.sourceId) || (blocks[i].id == value.targetId)) {
				dataconns.push({ "sourceId" : value.sourceId, "targetId" : value.targetId });
				break;
			}
		}
	});
	data.conns = dataconns;

	openProgressBar("Saving", "Saving application " + dataapplication.name + "...");
	$.post("saveApplication.php", { data : data }, function(data) {
		closeProgressBar();
		if (data.success == true) {
			openMsgbox("Application saved", "Your application has been saved.");
		} else {
		}
	}, "json");
	
	return false;
}

function openConfigDialog(block) {
	openProgressBar("Opening", "Opening block " + $(block).attr("type") + " configuration...");
	$.ajax({
		dataType : "jsonp",
		type : "POST",
		url : "getBlockConfig.php",
		data : { "id" : $(block).attr("id") },
		jsonp : "callback",
        jsonpCallback: "loadBlockConfig"
	});

}

function loadBlockConfig(xml) {
	closeProgressBar();

	$("#dialog-open-config").dialog("option", "title", $(xml).attr("id") + " configuration");
	$("#config-fields").empty();
	$(xml).find("params").each(function (index, value) {
		$(value).find("attr").each(function (index2, value2) {
			id = $(value2).attr("id");
			name = $(value2).attr("name");
			type = $(value2).attr("type");
			
			template = "#config-field-" + type + "-template";
			visname = "#config-field-" + type + "-name";
			editable = "#config-field-" + type + "-value";
			
			$("#config-fields").append($(template)[0].outerHTML);
			row = $("#config-fields").find(template);
			row.find(visname)[0].innerText = name;
			editobj = row.find(editable);
			editobj.id = id;

			if (type == "enumeration") {
				$(value2).find("option").each(function(index3, value3) {
					optid = $(value3).attr("id");
					optname = $(value3).attr("name");
					
					editobj.append($("#config-field-enumeration-value-template")[0].innerHTML);
					rd = editobj.find("#config-field-enumeration-value-id")[0];
					rd.id = optid;
					rd.name = optname;
					lb = editobj.find("#config-field-enumeration-value-name")[0];
					lb.id = newId();
					lb.htmlFor = optid;
					lb.innerText = optname;
				});
			}
			
			if (type == "array") {
				$(value2).find("option").each(function(index3, value3) {
					optid = $(value3).attr("id");
					optname = $(value3).attr("name");
					
					editobj.append($("#config-field-array-value-template")[0].innerHTML);
					it = editobj.find("#config-field-array-value-id")[0];
					it.id = newId();
					it.value = optid;
					it.innerHTML = optname;
				});
			}
		});
	});
	
	
//	$.each(data.data.params, function(index, value) {
//		if (value.type != "function") {
//			template = "#config-field-" + value.type + "-template";
//			name = "#config-field-" + value.type + "-name";
//			editable = "#config-field-" + value.type + "-value";
//			
//			$("#config-fields").append($(template)[0].outerHTML);
//			row = $("#config-fields").find(template);
//			row.find(name)[0].innerText = value.name;
//			editobj = row.find(editable);
//			editobj.id = value.id;
//			if (value.type == "radio") {
//				$.each(value.values, function(index2, value2) {
//					editobj.append($("#config-field-radio-value-template")[0].innerHTML);
//					rd = editobj.find("#config-field-radio-value-id")[0];
//					rd.id = value2.id;
//					rd.name = value.id;
//					lb = editobj.find("#config-field-radio-value-name")[0];
//					lb.id = newId();
//					lb.htmlFor = value2.id;
//					lb.innerText = value2.name;
//				});
//				
////						editobj.buttonset();
//			} else if (value.type == "checkbox") {
////						editobj.button();
//			}
//		}
//	});
//
//	$("#config-fields").append($("#config-field-functionsrow-template")[0].outerHTML);
//	row = $("#config-fields").find("#config-field-functionsrow-template");
//	$.each(data.data.params, function(index, value) {
//		if (value.type == "function") {
//			row.append($("#config-field-function-template")[0].outerHTML);
//			btn = row.find("#config-field-function-value");
//			btn[0].id = value.id;
//			btn.button({ label : value.name })
//			.click(function() {
//				openProgressBar("Executing", "Executing function " + $(this).button("option", "label") + "...");
//				$.post("executeApplicationFunction.php", { data : $(this).id }, function(data) {
//					closeProgressBar();
//					if (data.success == true) {
//						openMsgbox("Function executed", "The function " + $(this).button("option", "label") + " has been executed successfully.");
//					} else {
//					}
//				}, "json");
//			});
//		}
//	});

	$("#dialog-open-config").dialog("open");
}

$(document).ready(function() {
	// Toolbar button
    $("#button-new-composition").button( {
        text : false,
        label : "New composition",
        icons : {
            primary : "ui-icon-document"
        }
    }).click(function() {
        $("#dialog-new-composition").dialog("open");
        return false;
    });
    
    $("#button-open-composition").button( {
        text : false,
        label : "Open composition",
        icons : {
            primary : "ui-icon-folder-open"
        }
    }).click(function() {
        return getSavedCompositions();
    });
    
    $("#button-save-composition").button( {
        text : false,
        label : "Save composition",
        icons : {
            primary : "ui-icon-disk"
        }
    }).click(function() {
        return saveComposition();
    });
    
    $("#button-test-map").button( {
        text : false,
        label : "Test map",
        icons : {
            primary : "ui-icon-flag"
        }
    }).click(function() {
    	$("#dialog-test")
    		.dialog("option", "title", "Test map")
    		.dialog("open");

        return true;
    });
    
    // Dialogs
	$("#progress-bar").progressbar({
		value: 100
	});
    $("#dialog-progress-bar").dialog( {
        autoOpen : false,
        modal : true,
        draggable : true
    });
    
    $("#dialog-msgbox").dialog( {
        autoOpen : false,
        modal : true,
        draggable : true,
        buttons : {
            "Ok" : function() {
                $(this).dialog("close");
            }
        }
    });

    $("#dialog-msgbox-yesno").dialog( {
        autoOpen : false,
        modal : true,
        draggable : true,
        buttons : {
            "Ok" : function() {
                $(this).dialog("close");
            },
            "Cancel" : function() {
                $(this).dialog("close");
            }
        }
    });

    $("#dialog-new-composition").dialog( {
        autoOpen : false,
        title : "New composition",
        modal : true,
        draggable : true,
        buttons : {
            "Ok" : function() {
                $(this).dialog("close");
            },
            "Cancel" : function() {
                $(this).dialog("close");
            }
        },
        open : function() {
            $("#composition-name").focus();
        },
        close : function() {

        }
    });

    $("#dialog-open-composition").dialog( {
        autoOpen : false,
        title : "Opening",
        modal : true,
        draggable : true,
        buttons : {
            "Ok" : function() {
                $(this).dialog("close");
                var sel = $("#composition-select")[0];
                getComposition(sel.options[sel.selectedIndex].value, sel.options[sel.selectedIndex].text);
            },
            "Cancel" : function() {
                $(this).dialog("close");
            }
        },
        open : function() {
            $("#composition-select").focus();
        }
    });

    $("#dialog-open-config").dialog( {
        autoOpen : false,
        modal : true,
        draggable : true,
        buttons : {
            "Ok" : function() {
                $(this).dialog("close");
//                var sel = $("#application-select")[0];
//                getApplication(sel.options[sel.selectedIndex].value, sel.options[sel.selectedIndex].text);
            },
            "Cancel" : function() {
                $(this).dialog("close");
            }
        }
    });

    $("#dialog-test").dialog( {
        autoOpen : false,
        title : "Popularity",
        modal : true,
        draggable : true,
        position : "center",
        width : 800,
        height : 650,
        open : function(event, ui) {
    		drawRegionsMap();
    	},
        resizeStop : function(event, ui) {
    		drawRegionsMap(ui.size.width, ui.size.height);
    	},
        buttons : {
            "Ok" : function() {
                $(this).dialog("close");
            }
        }
    });
    
    // Charts
    
    // Tabs
	$("#tabs").tabs({
		panelTemplate : $("#workspace-template")[0].innerHTML,
		add : function(event, ui) {
        	$("#tabs").tabs("select", "#" + ui.panel.id);
		}
	});
	
    // Add an empty new composition
    addTab(newId(), "New composition");
});

// Google maps
function drawRegionsMap(w, h) {
	var data = new google.visualization.DataTable();
	data.addRows(4);
	data.addColumn('string', 'Country');
	data.addColumn('number', 'P');
	data.addColumn('number', 'Q');
	
//	data.setValue(0, 0, 'Germany');
//	data.setValue(0, 1, 200);
//	data.setValue(1, 0, 'United States');
//	data.setValue(1, 1, 300);
//	data.setValue(2, 0, 'Brazil');
//	data.setValue(2, 1, 400);
//	data.setValue(3, 0, 'Canada');
//	data.setValue(3, 1, 500);
//	data.setValue(4, 0, 'France');
//	data.setValue(4, 1, 600);
//	data.setValue(5, 0, 'RU');
//	data.setValue(5, 1, 700);

	data.setValue(0, 0, 'Germany');
	data.setValue(0, 1, 200);
	data.setValue(0, 2, 550);
	data.setValue(1, 0, 'Spain');
	data.setValue(1, 1, 300);
	data.setValue(1, 2, 650);
	data.setValue(2, 0, 'France');
	data.setValue(2, 1, 600);
	data.setValue(2, 2, 350);
	data.setValue(3, 0, 'Italy');
	data.setValue(3, 1, 500);
	data.setValue(3, 2, 250);

	var container = document.getElementById('test-map');
	if (typeof h == "undefined") {
		w = container.parentNode.clientWidth - 30;
		h = container.parentNode.clientHeight - 110;
	} else {
		w -= 20;
		h -= 90;
	}
	
	var options = {
		region : "150",
		width : w,
		height : h,
		colors : [ "#FFDDDD", "red" ]
	};

	var geochart = new google.visualization.GeoChart(container);
	geochart.draw(data, options);
};

google.load('visualization', '1', {
		'packages' : [ 'geochart' ]
	});

// Line connectors
jsPlumb.bind("ready", function() {
    // chrome fix.
    document.onselectstart = function () { return false; };
    jsPlumb.setRenderMode(jsPlumb.CANVAS);

	jsPlumb.setMouseEventsEnabled(true);
	jsPlumb.Defaults.Connector = [ "Bezier", { curviness : 75 } ];
	jsPlumb.Defaults.DragOptions = { cursor : "pointer", zIndex : 2000 };
	jsPlumb.Defaults.PaintStyle = { strokeStyle : "gray", lineWidth : 1 };
	jsPlumb.Defaults.Endpoint = "Dot";
	jsPlumb.Defaults.EndpointStyle = { radius : 5, fillStyle : "gray" };
	jsPlumb.Defaults.Anchors = [ "RightMiddle", "LeftMiddle" ];
	jsPlumb.Defaults.Overlays = [
		[
			"Arrow", 
			{ location : 1.0 },
			{ foldback : 0.8, fillStyle : "gray", length : 15, width : 8 }
		]
	];

//	var testWindows = [ $("#1314608470661"), $("#1314608473092"), $("#1314608475981")];
//	jsPlumb.draggable(testWindows, { containment: "#workspace" });
//	jsPlumb.addEndpoint(testWindows, sourceEndpoint);
//	jsPlumb.addEndpoint(testWindows, targetEndpoint);
	
//	jsPlumb.bind("jsPlumbConnection", function(connInfo) {
//		connInfo.connection.container = $("#workspace");
//	});
	
    jsPlumb.bind("click", function(conn) {
        openConfirm(
    		"Deleting connection",
    		"Delete connection from " + conn.sourceId + " to " + conn.targetId + "?",
    		function() {
    			jsPlumb.detach(conn);
        	}
        );
    });
});
