<div>
	<div class="box_generico box_centrato">
<?php
	if(!$successo){
		echo '<div class="error">';
		echo $GLOBALS['error_message'];
		echo '</div>';
	}else{
		echo '<div class="message">';
		echo $GLOBALS['message'];
		echo '</div>';
	}
?>
	<a href="index.php"><button class="button">Home</button></a>
	</div>
</div>
