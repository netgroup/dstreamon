<?php
	file_put_contents("data/".$_POST["data"]["application"]["id"], json_encode($_POST)); 

	echo json_encode(array("success"=>true));
//	echo json_encode(array("success"=>false,"errorcode"=>1,"Description"=>"General error"));
?>