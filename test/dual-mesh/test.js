/*
 * From https://github.com/redblobgames/dual-mesh
 * Copyright 2017 Red Blob Games <redblobgames@gmail.com>
 * License: Apache v2.0 <http://www.apache.org/licenses/LICENSE-2.0.html>
 */

'use strict';

let fs = require('fs');
let tape = require('tape');
let TriangleMesh = require('./');
let createMesh = require('./create');

tape("my test", function(test) {
    // Mesh spacing 5.0 lets me test the case of numRegions < (1<<16)
    // and numSides > (1<<16), which makes the two arrays different sizes
    let meshIn = createMesh({spacing: 50.0});
    
    test.end();
});