<?php
$_PCONN['cnt']++;
if (function_exists('zend_thread_id')) {
    echo "thread ", zend_thread_id(), "\n";
} else {
    echo "main\n";
}
