<?php
header("Content-Type: text/plain");
$xmlDoc = new DOMDocument();
$compositions = $xmlDoc->appendChild($xmlDoc->createElement("compositions"));

$dir = opendir("data/");
while($entryName = readdir($dir)) {
	if ($entryName[0] != ".") {
		$dirArray[] = $entryName;
	}
}
closedir($dir);

sort($dirArray);
foreach ($dirArray as &$value) {
	$compositions->appendChild($xmlDoc->createElement("composition", $value));
}

echo $_GET["callback"]."('".str_replace("\n", "", $xmlDoc->saveXML())."');";
?>