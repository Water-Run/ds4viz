<?php

declare(strict_types=1);

/**
 * ds4viz demo sample (PHP) - simple success
 * Outputs TOML IR to stdout by printing the generated trace.toml.
 *
 * This script expects ds4viz to be available via Composer autoload.
 * If your demo server runs code from /tmp, consider setting DS4VIZ_PHP_AUTOLOAD
 * to an absolute autoload.php path, or install ds4viz as a system-wide dependency.
 */

function requireDs4vizAutoload(): void
{
    $candidates = [];

    // 1) Explicit env override
    $env = getenv('DS4VIZ_PHP_AUTOLOAD');
    if (is_string($env) && $env !== '') {
        $candidates[] = $env;
    }

    // 2) Local vendor (if executed from a directory with vendor/)
    $candidates[] = __DIR__ . '/vendor/autoload.php';

    // 3) Common location for this repo layout: ~/Coding/ds4viz/library/php/vendor/autoload.php
    $home = getenv('HOME');
    if (is_string($home) && $home !== '') {
        $candidates[] = $home . '/Coding/ds4viz/library/php/vendor/autoload.php';
    }

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

$trace = 'trace.toml';
@unlink($trace);

Ds4viz::config(
    outputPath: $trace,
    title: 'PHP Stack Simple',
    author: 'WaterRun',
    comment: 'Simple success sample for demo server'
);

Ds4viz::stack(label: 'demo_stack_simple')
    ->run(function ($s): void {
        $s->push(10);
        $s->push(20);
        $s->push(30);
        $s->pop();
        $s->push(true);
        $s->push('ok');
    });

echo file_get_contents($trace);
