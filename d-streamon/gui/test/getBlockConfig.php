<?php
$data = '
<block id="'.$_POST["data"]["id"].'">
	<params id="source" name="source">
		<attr id="type" name="Type" type="enumeration">
			<option id="live" name="Live" />
			<option id="none" name="None" />
		</attr>
		<attr id="name" name="Name" type="array">
			<option id="eth0" name="eth0" />
			<option id="eth1" name="eth1" />
			<option id="eth2" name="eth2" />
			<option id="eth3" name="eth3" />
		</attr>
	</params>
</block>
';

echo $_GET["callback"]."('".str_replace("\n", "", $data)."');";
?>