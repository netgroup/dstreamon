<?php
	file_put_contents("data/".$_POST["data"]["composition"]["id"], json_encode($_POST)); 

	echo json_encode(array("code"=>0,"msg"=>""));
?>