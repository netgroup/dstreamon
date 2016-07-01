<?php
	sleep(2000);

	echo json_encode(array("success"=>true,"data"=>$_POST));
//	echo json_encode(array("success"=>false,"errorcode"=>1,"Description"=>"General error"));
?>