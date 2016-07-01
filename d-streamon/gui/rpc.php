<?php
if (! isset($_POST["params"]))
	$_POST["params"]=array();

$opts = array ('http' => array (
	'method'  => 'POST',
	'header'  => 'Content-type: application/json',
	'content' => json_encode($_POST)
));
$context = stream_context_create($opts);
if ($fp = fopen('http://demons.optenet.com:7080', 'r', false, $context)) {
	$response = '';
	while($row = fgets($fp)) {
		$response.= trim($row)."\n";
	}
	$arr = json_decode($response);
	echo $arr[0];
} else {
	throw new Exception('Unable to connect to server');
}
?>