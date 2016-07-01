$(document).ready(function() {
    $("#dialog-config").dialog( {
        autoOpen : false,
        title : "Configuration",
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

	// there's the gallery and the trash
	var $gallery = $( "#gallery" ),
		$trash = $( "#trash" );

	// let the gallery items be draggable
	$( "li", $gallery ).draggable({
		cancel: "a.ui-icon", // clicking an icon won't initiate dragging
		revert: "invalid", // when not dropped, the item will revert back to its initial position
		containment: $( "#demo-frame" ).length ? "#demo-frame" : "document", // stick to demo-frame if present
		helper: "clone",
		cursor: "move"
	});

	// let the trash be droppable, accepting the gallery items
	$trash.droppable({
		accept: "#gallery > li",
		activeClass: "ui-state-highlight",
		drop: function( event, ui ) {
			deleteImage( ui.draggable );
		}
	});

	// let the gallery be droppable as well, accepting items from the trash
	$gallery.droppable({
		accept: "#trash li",
		activeClass: "custom-state-active",
		drop: function( event, ui ) {
			recycleImage( ui.draggable );
		}
	});

	// image deletion function
	var recycle_icon = "<a href='link/to/recycle/script/when/we/have/js/off' title='Recycle this image' class='ui-icon ui-icon-refresh'>Recycle image</a>";
	function openConfig( $item ) {
		$("#dialog-config").dialog("open");
//		$item.fadeOut(function() {
//			var $list = $( "ul", $trash ).length ?
//				$( "ul", $trash ) :
//				$( "<ul class='gallery ui-helper-reset'/>" ).appendTo( $trash );
//
//			$item.find( "a.ui-icon-wrench" ).remove();
//			$item.append( recycle_icon ).appendTo( $list ).fadeIn(function() {
//				$item
//					.animate({ width: "48px" })
//					.find( "img" )
//						.animate({ height: "36px" });
//			});
//		});
	}

	// image recycle function
	var trash_icon = "<a href='link/to/trash/script/when/we/have/js/off' title='Delete this image' class='ui-icon ui-icon-wrench'>Delete image</a>";
	function recycleImage( $item ) {
		$item.fadeOut(function() {
			$item
				.find( "a.ui-icon-refresh" )
					.remove()
				.end()
				.css( "width", "196px")
				.append( trash_icon )
				.find( "img" )
					.css( "height", "72px" )
				.end()
				.appendTo( $gallery )
				.fadeIn();
		});
	}

	// image preview function, demonstrating the ui.dialog used as a modal window
	function viewLargerImage( $link ) {
		var src = $link.attr( "href" ),
			title = $link.siblings( "img" ).attr( "alt" ),
			$modal = $( "img[src$='" + src + "']" );

		if ( $modal.length ) {
			$modal.dialog( "open" );
		} else {
			var img = $( "<img alt='" + title + "' width='384' height='288' style='display: none; padding: 8px;' />" )
				.attr( "src", src ).appendTo( "body" );
			setTimeout(function() {
				img.dialog({
					title: title,
					width: 400,
					modal: true
				});
			}, 1 );
		}
	}

	// resolve the icons behavior with event delegation
	$( "ul.gallery > li" ).click(function( event ) {
		var $item = $( this ),
			$target = $( event.target );

		if ( $target.is( "a.ui-icon-wrench" ) ) {
			openConfig($item);
		} else if ( $target.is( "a.ui-icon-zoomin" ) ) {
			viewLargerImage( $target );
		} else if ( $target.is( "a.ui-icon-refresh" ) ) {
			recycleImage( $item );
		}

		return false;
	});
	
    drawChart();
    drawChart2();
    drawRegionsMap();
    drawGauge();
});

google.load("visualization", "1.0", {
	packages : [ "corechart", "geochart", "gauge" ]
});

function drawChart() {
	var data = new google.visualization.DataTable();
	data.addColumn("string", "Hour");
	data.addColumn("number", "NMDF");
	data.addRows([
	      		[ "1h", 250 ],
	    		[ "2h", 50 ],
	    		[ "3h", 50 ],
	    		[ "4h", 150 ],
	    		[ "5h", 120 ],
	    		[ "6h", 80 ],
	    		[ "7h", 40 ],
	    		[ "8h", 250 ],
	    		[ "9h", 375 ],
	    		[ "10h", 325 ],
	    		[ "11h", 120 ],
	    		[ "12h", 320 ],
	    		[ "13h", 50 ],
	    		[ "14h", 40 ],
	    		[ "15h", 100 ]
	]);

	// Set chart options
	var options = {
//		width : 400,
//		height : 300,
		"vAxis.minValue" : 0,
		"vAxis.maxValue" : 400,
		legend : "none"
	};

	// Instantiate and draw our chart, passing in some options.
	var chart = new google.visualization.LineChart(document.getElementById("chart_1"));
	chart.draw(data, options);
}

function drawChart2() {
	var data = new google.visualization.DataTable();
	data.addColumn("string", "Hour");
	data.addColumn("number", "NMDF");
	data.addColumn("number", "CP");
	data.addRows([
	      		[ "5/10/2011", 250, null ],
	    		[ "5/10/2011", 50, null ],
	    		[ "5/10/2011", 50, null ],
	    		[ "5/10/2011", 150, null ],
	    		[ "5/10/2011", 120, null ],
	    		[ "5/10/2011", 80, null ],
	    		[ "5/10/2011", 40, null ],
	    		[ "5/10/2011", 250, 350 ],
	    		[ "5/10/2011", 375, 380 ],
	    		[ "5/10/2011", 325, null ],
	    		[ "5/10/2011", 120, null ],
	    		[ "5/10/2011", 320, null ],
	    		[ "5/10/2011", 50, null ],
	    		[ "5/10/2011", 40, null ],
	    		[ "5/10/2011", 100, null ],
	    		[ "5/10/2011", 80, null ],
	    		[ "5/10/2011", 40, null ],
	    		[ "5/10/2011", 250, null ],
	    		[ "5/10/2011", 375, null ],
	    		[ "5/10/2011", 325, null ],
	    		[ "5/10/2011", 120, 230 ],
	    		[ "5/10/2011", 320, null ],
	    		[ "5/10/2011", 50, null ],
	    		[ "5/10/2011", 40, null ],
	    		
	      		[ "6/10/2011", 250, null ],
	    		[ "6/10/2011", 50, null ],
	    		[ "6/10/2011", 50, null ],
	    		[ "6/10/2011", 150, null ],
	    		[ "6/10/2011", 120, null ],
	    		[ "6/10/2011", 80, null ],
	    		[ "6/10/2011", 40, null ],
	    		[ "6/10/2011", 250, null ],
	    		[ "6/10/2011", 375, null ],
	    		[ "6/10/2011", 325, null ],
	    		[ "6/10/2011", 120, null ],
	    		[ "6/10/2011", 320, null ],
	    		[ "6/10/2011", 50, null ],
	    		[ "6/10/2011", 40, null ],
	    		[ "6/10/2011", 100, null ],
	    		[ "6/10/2011", 80, null ],
	    		[ "6/10/2011", 40, null ],
	    		[ "6/10/2011", 250, null ],
	    		[ "6/10/2011", 375, null ],
	    		[ "6/10/2011", 325, null ],
	    		[ "6/10/2011", 120, null ],
	    		[ "6/10/2011", 320, null ],
	    		[ "6/10/2011", 50, null ],
	    		[ "6/10/2011", 40, null ],
	    		
	      		[ "7/10/2011", 250, null ],
	    		[ "7/10/2011", 50, null ],
	    		[ "7/10/2011", 50, null ],
	    		[ "7/10/2011", 150, null ],
	    		[ "7/10/2011", 120, null ],
	    		[ "7/10/2011", 80, null ],
	    		[ "7/10/2011", 40, null ],
	    		[ "7/10/2011", 250, null ],
	    		[ "7/10/2011", 375, null ],
	    		[ "7/10/2011", 325, null ],
	    		[ "7/10/2011", 120, null ],
	    		[ "7/10/2011", 320, null ],
	    		[ "7/10/2011", 50, null ],
	    		[ "7/10/2011", 40, null ],
	    		[ "7/10/2011", 100, null ],
	    		[ "7/10/2011", 80, null ],
	    		[ "7/10/2011", 40, null ],
	    		[ "7/10/2011", 250, null ],
	    		[ "7/10/2011", 375, null ],
	    		[ "7/10/2011", 325, null ],
	    		[ "7/10/2011", 120, null ],
	    		[ "7/10/2011", 320, null ],
	    		[ "7/10/2011", 50, null ],
	    		[ "7/10/2011", 40, null ],
	    		
	      		[ "8/10/2011", 250, null ],
	    		[ "8/10/2011", 50, null ],
	    		[ "8/10/2011", 50, null ],
	    		[ "8/10/2011", 150, null ],
	    		[ "8/10/2011", 120, null ],
	    		[ "8/10/2011", 80, null ],
	    		[ "8/10/2011", 40, null ],
	    		[ "8/10/2011", 250, 350 ],
	    		[ "8/10/2011", 375, null ],
	    		[ "8/10/2011", 325, null ],
	    		[ "8/10/2011", 120, null ],
	    		[ "8/10/2011", 320, null ],
	    		[ "8/10/2011", 50, null ],
	    		[ "8/10/2011", 40, null ],
	    		[ "8/10/2011", 100, null ],
	    		[ "8/10/2011", 80, null ],
	    		[ "8/10/2011", 40, null ],
	    		[ "8/10/2011", 250, null ],
	    		[ "8/10/2011", 375, null ],
	    		[ "8/10/2011", 325, null ],
	    		[ "8/10/2011", 120, null ],
	    		[ "8/10/2011", 320, null ],
	    		[ "8/10/2011", 50, 20 ],
	    		[ "8/10/2011", 40, null ],
	    	
	      		[ "9/10/2011", 250, null ],
	    		[ "9/10/2011", 50, null ],
	    		[ "9/10/2011", 50, null ],
	    		[ "9/10/2011", 150, null ],
	    		[ "9/10/2011", 120, null ],
	    		[ "9/10/2011", 80, null ],
	    		[ "9/10/2011", 40, null ],
	    		[ "9/10/2011", 250, null ],
	    		[ "9/10/2011", 375, null ],
	    		[ "9/10/2011", 325, null ],
	    		[ "9/10/2011", 120, null ],
	    		[ "9/10/2011", 320, null ],
	    		[ "9/10/2011", 50, null ],
	    		[ "9/10/2011", 40, null ],
	    		[ "9/10/2011", 100, null ],
	    		[ "9/10/2011", 80, null ],
	    		[ "9/10/2011", 40, null ],
	    		[ "9/10/2011", 250, null ],
	    		[ "9/10/2011", 375, null ],
	    		[ "9/10/2011", 325, null ],
	    		[ "9/10/2011", 120, null ],
	    		[ "9/10/2011", 320, null ],
	    		[ "9/10/2011", 50, null ],
	    		[ "9/10/2011", 40, null ],
	    		
	      		[ "10/10/2011", 250, null ],
	    		[ "10/10/2011", 50, null ],
	    		[ "10/10/2011", 50, null ],
	    		[ "10/10/2011", 150, null ],
	    		[ "10/10/2011", 120, null ],
	    		[ "10/10/2011", 80, null ],
	    		[ "10/10/2011", 40, null ],
	    		[ "10/10/2011", 250, null ],
	    		[ "10/10/2011", 375, null ],
	    		[ "10/10/2011", 325, null ],
	    		[ "10/10/2011", 120, null ],
	    		[ "10/10/2011", 320, null ],
	    		[ "10/10/2011", 50, null ],
	    		[ "10/10/2011", 40, null ],
	    		[ "10/10/2011", 100, null ],
	    		[ "10/10/2011", 80, null ],
	    		[ "10/10/2011", 40, null ],
	    		[ "10/10/2011", 250, null ],
	    		[ "10/10/2011", 375, null ],
	    		[ "10/10/2011", 325, null ],
	    		[ "10/10/2011", 120, null ],
	    		[ "10/10/2011", 320, null ],
	    		[ "10/10/2011", 50, null ],
	    		[ "10/10/2011", 40, null ],
	    		
	      		[ "11/10/2011", 250, null ],
	    		[ "11/10/2011", 50, null ],
	    		[ "11/10/2011", 50, null ],
	    		[ "11/10/2011", 150, null ],
	    		[ "11/10/2011", 120, null ],
	    		[ "11/10/2011", 80, null ],
	    		[ "11/10/2011", 40, null ],
	    		[ "11/10/2011", 250, null ],
	    		[ "11/10/2011", 375, null ],
	    		[ "11/10/2011", 325, null ],
	    		[ "11/10/2011", 120, null ],
	    		[ "11/10/2011", 320, null ],
	    		[ "11/10/2011", 50, null ],
	    		[ "11/10/2011", 40, null ],
	    		[ "11/10/2011", 100, null ],
	    		[ "11/10/2011", 80, null ],
	    		[ "11/10/2011", 40, null ],
	    		[ "11/10/2011", 250, null ],
	    		[ "11/10/2011", 375, null ],
	    		[ "11/10/2011", 325, null ],
	    		[ "11/10/2011", 120, null ],
	    		[ "11/10/2011", 320, null ],
	    		[ "11/10/2011", 50, null ],
	    		[ "11/10/2011", 40, null ],
	    		
	      		[ "12/10/2011", 250, null ],
	    		[ "12/10/2011", 50, null ],
	    		[ "12/10/2011", 50, null ],
	    		[ "12/10/2011", 150, null ],
	    		[ "12/10/2011", 120, null ],
	    		[ "12/10/2011", 80, null ],
	    		[ "12/10/2011", 40, null ],
	    		[ "12/10/2011", 250, null ],
	    		[ "12/10/2011", 375, null ],
	    		[ "12/10/2011", 325, null ],
	    		[ "12/10/2011", 120, null ],
	    		[ "12/10/2011", 320, null ],
	    		[ "12/10/2011", 50, null ],
	    		[ "12/10/2011", 40, null ],
	    		[ "12/10/2011", 100, null ],
	    		[ "12/10/2011", 80, null ],
	    		[ "12/10/2011", 40, null ],
	    		[ "12/10/2011", 250, null ],
	    		[ "12/10/2011", 375, null ],
	    		[ "12/10/2011", 425, null ],
	    		[ "12/10/2011", 120, null ],
	    		[ "12/10/2011", 320, null ],
	    		[ "12/10/2011", 50, null ],
	    		[ "12/10/2011", 40, null ],
	    		
	      		[ "13/10/2011", 250, null ],
	    		[ "13/10/2011", 50, null ],
	    		[ "13/10/2011", 50, null ],
	    		[ "13/10/2011", 150, null ],
	    		[ "13/10/2011", 120, null ],
	    		[ "13/10/2011", 80, null ],
	    		[ "13/10/2011", 40, null ],
	    		[ "13/10/2011", 250, null ],
	    		[ "13/10/2011", 375, null ],
	    		[ "13/10/2011", 325, null ],
	    		[ "13/10/2011", 120, null ],
	    		[ "13/10/2011", 320, null ],
	    		[ "13/10/2011", 50, null ],
	    		[ "13/10/2011", 40, null ],
	    		[ "13/10/2011", 100, null ],
	    		[ "13/10/2011", 80, null ],
	    		[ "13/10/2011", 40, null ],
	    		[ "13/10/2011", 250, null ],
	    		[ "13/10/2011", 375, null ],
	    		[ "13/10/2011", 325, null ],
	    		[ "13/10/2011", 120, null ],
	    		[ "13/10/2011", 320, null ],
	    		[ "13/10/2011", 50, null ],
	    		[ "13/10/2011", 40, null ],
	    		
	      		[ "14/10/2011", 250, null ],
	    		[ "14/10/2011", 50, null ],
	    		[ "14/10/2011", 50, null ],
	    		[ "14/10/2011", 150, null ],
	    		[ "14/10/2011", 120, null ],
	    		[ "14/10/2011", 80, null ],
	    		[ "14/10/2011", 40, null ],
	    		[ "14/10/2011", 250, null ],
	    		[ "14/10/2011", 375, null ],
	    		[ "14/10/2011", 325, null ],
	    		[ "14/10/2011", 120, null ],
	    		[ "14/10/2011", 320, null ],
	    		[ "14/10/2011", 50, null ],
	    		[ "14/10/2011", 40, null ],
	    		[ "14/10/2011", 100, null ],
	    		[ "14/10/2011", 80, null ],
	    		[ "14/10/2011", 40, null ],
	    		[ "14/10/2011", 250, null ],
	    		[ "14/10/2011", 375, null ],
	    		[ "14/10/2011", 325, null ],
	    		[ "14/10/2011", 120, null ],
	    		[ "14/10/2011", 320, null ],
	    		[ "14/10/2011", 50, null ],
	    		[ "14/10/2011", 40, null ],
	    		
	      		[ "15/10/2011", 250, null ],
	    		[ "15/10/2011", 50, null ],
	    		[ "15/10/2011", 50, null ],
	    		[ "15/10/2011", 150, null ],
	    		[ "15/10/2011", 120, null ],
	    		[ "15/10/2011", 80, null ],
	    		[ "15/10/2011", 40, null ],
	    		[ "15/10/2011", 250, null ],
	    		[ "15/10/2011", 375, null ],
	    		[ "15/10/2011", 325, null ],
	    		[ "15/10/2011", 120, null ],
	    		[ "15/10/2011", 320, null ],
	    		[ "15/10/2011", 50, null ],
	    		[ "15/10/2011", 40, null ],
	    		[ "15/10/2011", 100, null ],
	    		[ "15/10/2011", 80, null ],
	    		[ "15/10/2011", 40, null ],
	    		[ "15/10/2011", 250, null ],
	    		[ "15/10/2011", 375, null ],
	    		[ "15/10/2011", 325, null ],
	    		[ "15/10/2011", 120, null ],
	    		[ "15/10/2011", 320, null ],
	    		[ "15/10/2011", 50, null ],
	    		[ "15/10/2011", 40, null ],
	    		
	      		[ "16/10/2011", 250, null ],
	    		[ "16/10/2011", 50, null ],
	    		[ "16/10/2011", 50, null ],
	    		[ "16/10/2011", 150, null ],
	    		[ "16/10/2011", 120, null ],
	    		[ "16/10/2011", 80, null ],
	    		[ "16/10/2011", 40, null ],
	    		[ "16/10/2011", 250, null ],
	    		[ "16/10/2011", 375, null ],
	    		[ "16/10/2011", 325, 130 ],
	    		[ "16/10/2011", 120, null ],
	    		[ "16/10/2011", 320, null ],
	    		[ "16/10/2011", 50, null ],
	    		[ "16/10/2011", 40, null ],
	    		[ "16/10/2011", 100, null ],
	    		[ "16/10/2011", 80, null ],
	    		[ "16/10/2011", 40, null ],
	    		[ "16/10/2011", 250, null ],
	    		[ "16/10/2011", 375, null ],
	    		[ "16/10/2011", 325, null ],
	    		[ "16/10/2011", 120, null ],
	    		[ "16/10/2011", 320, null ],
	    		[ "16/10/2011", 50, null ],
	    		[ "16/10/2011", 40, null ]
	]);

	// Set chart options
	var options = {
//		width : 400,
//		height : 300,
		series : {
			0 : {
				lineWidth : 1
			},
			1 : {
				color : 'red',
				lineWidth : 0,
				pointSize : 5
			}
		},
		"vAxis.minValue" : 0,
		"vAxis.maxValue" : 400,
		legend : "none"
	};

	// Instantiate and draw our chart, passing in some options.
	var chart = new google.visualization.LineChart(document.getElementById("chart_4"));
	chart.draw(data, options);
}

function drawRegionsMap(w, h) {
	var data = new google.visualization.DataTable();
	data.addRows(4);
	data.addColumn('string', 'Country');
	data.addColumn('number', 'P');
	
	data.setValue(0, 0, 'Germany');
	data.setValue(0, 1, 200);
	data.setValue(1, 0, 'Spain');
	data.setValue(1, 1, 300);
	data.setValue(2, 0, 'France');
	data.setValue(2, 1, 600);
	data.setValue(3, 0, 'Italy');
	data.setValue(3, 1, 500);

//	var container = document.getElementById('test-map');
//	if (typeof h == "undefined") {
//		w = container.parentNode.clientWidth - 30;
//		h = container.parentNode.clientHeight - 110;
//	} else {
//		w -= 20;
//		h -= 90;
//	}
	
	var options = {
		region : "150",
//		width : w,
//		height : h,
		colors : [ "#FFDDDD", "red" ]
	};

//	var geochart = new google.visualization.GeoChart(container);
	var geochart = new google.visualization.GeoChart(document.getElementById("chart_2"));

	geochart.draw(data, options);
};

function drawGauge() {
  // Create and populate the data table.
	var data = new google.visualization.DataTable();
	data.addColumn('string', 'Label');
	data.addColumn('number', 'Value');
	data.addRows(3);
	data.setValue(0, 0, 'Danger');
	data.setValue(0, 1, 63);

	var options = {
		redFrom : 90,
		redTo : 100,
		yellowFrom : 70,
		yellowTo : 90,
		greenFrom : 0,
		greenTo : 70,
		majorTicks : [ 0, 20, 40, 60, 80, 100 ],
		minorTicks : 4
	};

	// Create and draw the visualization.
	new google.visualization.Gauge(document.getElementById("chart_3")).draw(
			data, options);
};
