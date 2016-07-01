var blocksConfig = {};
var selectedEndPoint = null;
var lastId = 0;

function newId() {
	var today = new Date;
	var id = today.getTime();
	if (id == lastId) id++;
	lastId = id;
	return id;
}

function openGeneralDlg(g, t, d, f) {
	var dlg = $(g);
	
	if (t != null)
		dlg.dialog("option", "title", t);
	if (d != null)
		$(g + "-description")[0].innerHTML = d;
	
	var buttons = {};
	if (f != null) {
		buttons["Ok"] = function() {
	    	$(this).dialog("close");
	    	if ((f != undefined) && (f["Ok"] != undefined))
	    		f["Ok"]();
		};
	    if (f != undefined) {
	        for (var btn in f) {
	        	if ((btn != "Ok") && (btn != "Cancel")) {
	        		buttons[btn] = function(ev) {
	        	    	$(this).dialog("close");
	    	    		f[ev.currentTarget.textContent]();
	        		};
	        	}
	        }
	    }
	    buttons["Cancel"] = function() {
			$(this).dialog("close");
	    	if ((f != undefined) && (f["Cancel"] != undefined))
	    		f["Cancel"]();
		};
	}
	
	dlg.dialog("option", "buttons", buttons);
	dlg.dialog("open");
}

function openProgressBar(t, d) {
	openGeneralDlg("#dialog-progress-bar", t, d, null);
}

function closeProgressBar() {
	$("#dialog-progress-bar").dialog("close");
}

function openMsgbox(t, d) {
	openGeneralDlg("#dialog-msgbox", t, d);
}

function openConfirm(t, d, yesfunction) {
	openGeneralDlg("#dialog-msgbox-yesno", t, d, {
		"Ok" : yesfunction
	});
}

function addConfigField(type, id, name, value, disabled, options) {
	template = "#config-field-" + type + "-template";
	visname = "#config-field-" + type + "-name";
	editable = "#config-field-" + type + "-value";
			
	$("#dialog-config-fields").append($(template)[0].outerHTML);
	row = $("#dialog-config-fields").find(template)[0];
	$(row).find(visname)[0].innerHTML = name;
	editobj = $(row).find(editable)[0];
	editobj.id = id;

	// Options for enumerations and disabling it
	if (type == "enumeration") {
		$(options).each(function(idxOpt, valOpt) {
			optid = $(valOpt).attr("id");
			optname = $(valOpt).attr("name");
			
			$(editobj).append($("#config-field-enumeration-value-template")[0].innerHTML);
			rd = $(editobj).find("#config-field-enumeration-value-id")[0];
			rd.id = optid;
			rd.name = id;
			rd.disabled = disabled;
			lb = $(editobj).find("#config-field-enumeration-value-name")[0];
			lb.id = newId();
			lb.htmlFor = optid;
			lb.innerHTML = optname;
		});
	} else {
		editobj.disabled = disabled;
	}
	
	// Setting values
	try {
		if (type == "textarea")
			editobj.innerHTML = value;
		else if (type == "checkbox")
			editobj.checked = value;
		else if (type == "enumeration")
			$(editobj).find("#" + value)[0].checked = true;
		else
			editobj.value = value;
	} catch (e) {
	};

//			
//			if (type == "array") {
//				$(value2).find("option").each(function(index3, value3) {
//					optid = $(value3).attr("id");
//					optname = $(value3).attr("name");
//					
//					editobj.append($("#config-field-array-value-template")[0].innerHTML);
//					it = editobj.find("#config-field-array-value-id")[0];
//					it.id = newId();
//					it.value = optid;
//					it.innerHTML = optname;
//				});
//			}
//		});
//	});
}

function blockConfigDlg(block) {
	var type = $(block).attr("type");
	$("#dialog-config-id")[0].innerHTML = block.id;
	$("#dialog-config-fields").empty();
//	addConfigField("textarea", "params_schema", "Params schema", blocksConfig[type].params_schema, true);
	addConfigField("enumeration", "scheduling_type", "Scheduling type", blocksConfig[type].scheduling_type, 
			(blocksConfig[type].scheduling_type != "both"),
			[{
				id : "active",
				name : "Active"
			},{
				id : "passive",
				name : "Passive"
			}]);
	addConfigField("checkbox", "thread_exclusive", "Thread exclusive", blocksConfig[type].thread_exclusive, true);

	openGeneralDlg("#dialog-config", type + " configuration", blocksConfig[type].human_desc, {
		"Delete" : function() {
			deleteBlock($("#dialog-config-id")[0].innerHTML);
		}
	});
}

function gateConfigDlg(endp) {
	selectedEndPoint = endp;
	var dir = (endp.isSource ? "output" : "input");
	var type = $(endp.element[0]).attr("type");
	$("#dialog-config-id")[0].innerHTML = endp.id;
	$("#dialog-config-fields").empty();
	
	openGeneralDlg("#dialog-config", type + " " + dir + " gate configuration", "", {
		"Clean" : function() {
			selectedEndPoint.detachAll();
		},
		"Cancel" : function() {
			selectedEndPoint = null;
		}
	});
}

function addBlock(id, idComp, name, type, posTop, posLeft) {
	var idworkspace = "#" + idComp + "-workspace";
	objworkspace = $(idworkspace),
	blockcontent = $("#block-template")[0].innerHTML;
	objworkspace.append(blockcontent);
	
	var newblock = objworkspace.find("#block")[0];
	newblock.id = id;
	$(newblock).attr("type", type);
	newblock = $("#" + id);
	newblock.css("position", "inherit");
	newblock.css("top", posTop - objworkspace.offset().top - 1);
	newblock.css("left", posLeft - objworkspace.offset().left - 1);
	newblock.dblclick(function() {
		blockConfigDlg(this);
	});
	
	jsPlumb.draggable(newblock, { containment: idworkspace });
	if (blocksConfig[type].inputs.length > 0) {
		jsPlumb.addEndpoint(newblock, {
			isTarget : true,
			anchor : "LeftMiddle",
			maxConnections : blocksConfig[type].inputs.length,
			endpoint : [ "Image", {
				src: "/demons/images/yellow" +
					(blocksConfig[type].inputs.length <= 5 ? blocksConfig[type].inputs.length : "5+") +
					".png"
			}]
		});
	}
	if (blocksConfig[type].outputs.length > 0) {
		jsPlumb.addEndpoint(newblock, {
			isSource : true,
			anchor : "RightMiddle",
			maxConnections : blocksConfig[type].outputs.length,
			endpoint : [ "Image", {
				src: "/demons/images/yellow" +
					(blocksConfig[type].outputs.length <= 5 ? blocksConfig[type].outputs.length : "5+") +
					".png"
			}]
		});
	}
	
	var newblockname = newblock.find("#block-name");
	newblockname.id = "block-name-" + id;
	newblockname[0].innerHTML = name;
}

function deleteBlock(id) {
	var block = $("#" + id);
	
	jsPlumb.removeAllEndpoints(block);
	block.remove();
}

function addTab(id, t) {
    $("#tabs").tabs("add", "#tabs-" + id, t);
	
	// Retrieve blocks list
	openProgressBar("Loading...", "Getting blocks list");
	server.get_blocks_list(function(data) {
		closeProgressBar();
		if (data._ReturnValue__code == 0) {
			openProgressBar("Loading...", "Loading blocks configuration...");
			$("#progress-bar").progressbar({ value : 0 });
			var total = data._ReturnValue__value.length;
			var cont = 0;
			
			$(data._ReturnValue__value).each(function(iList, iValue) {
				$("#tabs-" + id).find("#blocks-section").append($("#block-template")[0].innerHTML);
				obj = $("#tabs-" + id).find("#blocks-section").find("#block")[0];
				obj.id = iValue;
				obj.type = iValue;
				$(obj).attr("type", iValue);
				$(obj).find("#block-name")[0].innerHTML = iValue;
				
				// Retrieve block config
				if (blocksConfig[iValue] == undefined) {
					server.get_blocks_info([iValue], function(cfg) {
						if (cfg._ReturnValue__code == 0) {
							$(cfg._ReturnValue__value).each(function(iCfg, vCfg) {
								var aIn = [], aOut = [];
								$(vCfg._BlockInfo__gates).each(function(iGate, vGate) {
									var gate = {
										msg_type : vGate._GateInfo__msg_type,
										name : vGate._GateInfo__name
									};
									if (vGate._GateInfo__type == "input")
										aIn.push(gate);
									else if (vGate._GateInfo__type == "output")
										aOut.push(gate);
								});
								
								blocksConfig[iValue] = {
									human_desc : vCfg._BlockInfo__human_desc,
									inputs : aIn,
									outputs : aOut,
									params_schema : vCfg._BlockInfo__params_schema,
									scheduling_type : "active", //vCfg._BlockInfo__is_active,
//									scheduling_type : vCfg._BlockInfo__scheduling_type,
									thread_exclusive : vCfg._BlockInfo__thread_exclusive
								};
							});
						} else {
							openMsgbox("Critical error", "Error " + cfg._ReturnValue__code + ": " + cfg._ReturnValue__msg);
						}
						
						cont++;
						$("#progress-bar").progressbar({ value : 100 * cont / total });
						if (cont == total)
							closeProgressBar();
					});
				} else {
					console.log("Already loaded");
					
					cont++;
					$("#progress-bar").progressbar({ value : 100 * cont / total });
					if (cont == total)
						closeProgressBar();
				}
			});
			
			var newtab = $("#tabs-" + id);
			$.each(["drag-container","workspace",/*"start",*/"blocks-available","blocks-accordion"], //,"blocks-section"
				function(index, value) {
					newtab.find("#" + value)[0].id = id + "-" + value;
				}
			);
			
//			var startBox = [ $("#" + id + "-start") ];
//			jsPlumb.draggable(startBox, { containment: "#" + id + "-workspace" });
//			jsPlumb.addEndpoint(startBox, sourceEndpoint);
			
			// Available blocks
			$("#" + id + "-blocks-accordion").accordion({
				fillSpace: true,
				icons: {
					header: "ui-icon-circle-arrow-e",
					headerSelected: "ui-icon-circle-arrow-s"
				}
			});

			$("#" + id + "-blocks-available div .block").draggable({
				appendTo : "#" + id + "-drag-container",
				containment: "#" + id + "-drag-container",
//				stack: "#" + id + "-blocks-available div",
				helper : "clone",
				stop : function(event, ui) {
					if (ui.position.left < $("#" + id + "-workspace")[0].offsetWidth - ui.helper[0].clientWidth) {
						addBlock(newId(), id,
								ui.helper[0].innerHTML,
								ui.helper[0].attributes["type"].value,
								ui.offset.top, ui.offset.left);
					}
				}
			});
		} else {
			openMsgbox("Critical error", "Error " + data._ReturnValue__code + ": " + data._ReturnValue__msg);
			
		    $("#tabs").tabs("remove", $("#tabs").tabs("length") - 1);
		}
	});
}

//function getSavedCompositions() {
//	openProgressBar("Opening", "Get list of saved compositions...");
//	$.ajax({
//		dataType : "jsonp",
//		type : "POST",
//		url : "getSavedCompositions.php",
//		jsonp : "callback",
//        jsonpCallback: "loadSavedCompositions"
//	});
//	
//	return false;
//}
//
//function loadSavedCompositions(xml) {
//	closeProgressBar();
//	
//	$("#composition-select")[0].options.length = 0;
//	$(xml).find("composition").each(function() {
//		var element = document.createElement("option");
//		element.text = $(this).text();
//		element.value = $(this).text();
//		$("#composition-select")[0].add(element, null);
//	});
//    $("#dialog-open-composition").dialog("open");
//}
//
//function getComposition(id, t) {
//	openProgressBar("Opening", "Opening composition " + t + "...");
//	$.ajax({
//		dataType : "jsonp",
//		type : "POST",
//		url : "getComposition.php",
//		data : { "id" : id },
//		jsonp : "callback",
//        jsonpCallback: "loadComposition"
//	});
//	
//	return false;
//}
//
//function loadComposition(xml) {
//	closeProgressBar();
//	
//	addTab($(xml).attr("id"), $(xml).attr("id"));
//	var objworkspace = $("#" + $(xml).attr("id") + "-workspace");
//	$(xml).find("block").each(function (index, value) {
//		addBlock(
//			$(value).attr("id"),
//			$(xml).attr("id"),
//			$(value).attr("id"),
//			$(value).attr("type"),
//			objworkspace.offset().top + 1 + index * 50,
//			objworkspace.offset().left + 1 + index * 50
//		);
//	});
//	
//	$(xml).find("connection").each(function (index, value) {
//		jsPlumb.connect({
//			source : $(value).attr("src_block"),
//			target : $(value).attr("dst_block")
//		});
//	});
////	
////	  $(xml).find("Tutorial").each(function()
////			  {
////			    $("#output").append($(this).attr("author") + "<br />");
////			  });
////	
////	$.each(conns, function(index, value) {
////		jsPlumb.connect({
////			source : value.sourceId,
////			target : value.targetId
////		});
////	});
//	
//	return false;
//}

function saveComposition() {
	var data = {};
	
	var dataComposition = {
		id : $("#tabs").find(".ui-tabs-panel")[($("#tabs").tabs("option", "selected"))].id.substr(5),
		name : $("#tabs").find(".ui-tabs-selected")[0].innerText
	};
	data.composition = dataComposition;
	
	var dataBlocks = [];
	var blocks = $("#" + dataComposition.id + "-workspace .block");
	$.each(blocks, function(index, value) {
		dataBlocks.push({
			"id" : value.id,
			"name" : value.innerText,
			"type" : $(value).attr("type"),
			"top" : value.offsetTop,
			"left" : value.offsetLeft
		});
	});
	data.blocks = dataBlocks;
	
	var dataConns = [];
	var conns = jsPlumb.getConnections();
	$.each(conns, function(index, value) {
		for (i = 0; i < blocks.length; ++i) {
			if ((blocks[i].id == value.sourceId) || (blocks[i].id == value.targetId)) {
				dataConns.push({
					"sourceId" : value.sourceId,
					"targetId" : value.targetId
				});
				break;
			}
		}
	});
	data.conns = dataConns;

	openProgressBar("Saving", "Saving composition " + dataComposition.name + "...");
	$.post("saveComposition.php", { data : data }, function(data) {
		closeProgressBar();
		if (data.code == 0) {
			openMsgbox("Composition saved", "Your composition has been saved.");
		} else {
		}
	}, "json");
	
	return false;
}

$(document).ready(function() {
	// Toolbar buttons
    $("#button-new-composition").button({
    	disabled : false,
        text : false,
        label : "New composition",
        icons : {
            primary : "ui-icon-document"
        }
    }).click(function() {
    	$("#dialog-new-composition-id")[0].value = newId();
    	openGeneralDlg("#dialog-new-composition", null, null, {
    		"Ok" : function() {
    			addTab($("#dialog-new-composition-id")[0].value, $("#dialog-new-composition-name")[0].value);
    		}
    	});
    	return false;
    });
    
    $("#button-open-composition").button({
    	disabled : true,
        text : false,
        label : "Open composition",
        icons : {
            primary : "ui-icon-folder-open"
        }
    }).click(function() {
//        return getSavedCompositions();
    });
    
    $("#button-save-composition").button({
    	disabled : false,
        text : false,
        label : "Save composition",
        icons : {
            primary : "ui-icon-disk"
        }
    }).click(function() {
        return saveComposition();
    });
    
    // Dialogs
	$("#progress-bar").progressbar({
		value: 100
	});
    $("#dialog-progress-bar").dialog({
        autoOpen : false,
        modal : true,
        draggable : true
    });
    
    $("#dialog-msgbox").dialog({
        autoOpen : false,
        modal : true,
        draggable : true,
        buttons : {
            "Ok" : function() {
                $(this).dialog("close");
            }
        }
    });

    $("#dialog-msgbox-yesno").dialog({
        autoOpen : false,
        modal : true,
        draggable : true/*,
        buttons : {
            "Ok" : function() {
                $(this).dialog("close");
            },
            "Cancel" : function() {
                $(this).dialog("close");
            }
        }*/
    });

    $("#dialog-new-composition").dialog({
        autoOpen : false,
        title : "New composition",
        modal : true,
        draggable : true,
        open : function() {
    		$("#dialog-new-composition-name")[0].focus();
        },
        close : function() {

        }
    });
//
//    $("#dialog-open-composition").dialog( {
//        autoOpen : false,
//        title : "Opening",
//        modal : true,
//        draggable : true,
//        buttons : {
//            "Ok" : function() {
//                $(this).dialog("close");
//                var sel = $("#composition-select")[0];
//                getComposition(sel.options[sel.selectedIndex].value, sel.options[sel.selectedIndex].text);
//            },
//            "Cancel" : function() {
//                $(this).dialog("close");
//            }
//        },
//        open : function() {
//            $("#composition-select").focus();
//        }
//    });

    $("#dialog-config").dialog({
		autoOpen : false,
		modal : true,
		draggable : true
	});

    // Tabs
	$("#tabs").tabs({
		panelTemplate : $("#workspace-template")[0].innerHTML,
		add : function(event, ui) {
        	$("#tabs").tabs("select", "#" + ui.panel.id);
		}
	});
	
    // Add an empty new composition
//    addTab(newId(), "New composition");
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
	jsPlumb.Defaults.Endpoint = [ "Image", { src : "/demons/images/green.png" } ];
	jsPlumb.Defaults.Anchors = [ "RightMiddle", "LeftMiddle" ];
	jsPlumb.Defaults.Overlays = [
		[
			"Arrow", 
			{ location : 1.0 },
			{ foldback : 0.8, fillStyle : "gray", length : 15, width : 8 }
		]
	];

    jsPlumb.bind("jsPlumbConnection", function(conn) {
    	$([ conn.sourceEndpoint, conn.targetEndpoint ]).each(function (idx, val) {
    		var search = val.canvas.src.match(/yellow*./);
    		if (search != null) {
    			var number = search[0].substr(6);
	    		if (val.connectorSelector() != null) {
	    			val.canvas.src = "/demons/images/green" + number + ".png";
	    		}
    		}
    	});
    });

    jsPlumb.bind("jsPlumbConnectionDetached", function(conn) {
    	$([ conn.sourceEndpoint, conn.targetEndpoint ]).each(function (idx, val) {
    		var search = val.canvas.src.match(/green*./);
    		if (search != null) {
	    		var number = search[0].substr(5);
				val.canvas.src = "/demons/images/yellow" + number + ".png";
    		}
    	});
    });
    
    jsPlumb.bind("dblclick", function(conn) {
        openConfirm(
    		"Deleting connection",
    		"Delete connection from " + conn.sourceId + " to " + conn.targetId + "?",
    		function() {
    			jsPlumb.detach(conn);
        	}
        );
    });

    jsPlumb.bind("endpointDblClick", function(endp) {
    	gateConfigDlg(endp);
    });
});
