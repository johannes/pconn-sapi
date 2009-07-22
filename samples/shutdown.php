<?php
echo "shutdown ".(function_exists('zend_thread_id') ? zend_thread_id()." - " : '') .$_PCONN['cnt']."\n";
var_dump($_PCONN);
