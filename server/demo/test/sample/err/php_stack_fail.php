<?php

declare(strict_types=1);

/**
 * ds4viz demo sample (PHP) - failure case
 * Intentionally triggers an error (pop on empty stack).
 * The error should be reflected in TOML ([error] section).
 */

function requireDs4vizAutoload(): void
{
    $candidates = [];
    $env = getenv('DS4VIZ_PHP_AUTOLOAD');
    if (is_string($env) && $env !== '') $candidates[] = $env;
    $candidates[] = __DIR__ . '/vendor/autoload.php';
    $home = getenv('HOME');
    if (is_string($home) && $home !== '') $candidates[] = $home . '/Coding/ds4viz/library/php/vendor/autoload.php';

    foreach ($candidates as $path) {
        if (is_string($path) && file_exists($path)) {
            require_once $path;
            return;
        }
    }
    throw new RuntimeException('Cannot locate Composer autoload.php for ds4viz. Set DS4VIZ_PHP_AUTOLOAD.');
}

requireDs4vizAutoload();

use Ds4viz\Ds4viz;
use Ds4viz\Exception\StructureException;

$trace = 'trace.toml';
@unlink($trace);

Ds4viz::config(
    outputPath: $trace,
    title: 'PHP Stack Fail',
    author: 'WaterRun',
    comment: 'Failure sample: pop from empty stack'
);

try {
    Ds4viz::stack(label: 'demo_stack_fail')
        ->run(function ($s): void {
            // Empty stack -> should throw StructureException
            $s->pop();
        });
} catch (StructureException $e) {
    // Expected; ds4viz should still generate trace.toml with [error].
}

if (file_exists($trace)) {
    echo file_get_contents($trace);
} else {
    echo "[error]\nmessage = \"trace.toml not generated; check ds4viz install\"\n";
}
