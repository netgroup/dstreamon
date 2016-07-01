<?php
$data = file_get_contents("data/".$_POST["id"]);

echo $_GET["callback"]."('".str_replace("\n", "", $data)."');";
?>