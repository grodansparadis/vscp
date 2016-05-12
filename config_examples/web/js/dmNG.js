//
// VSCP decision matrix Next Generation
// Copyright (c) 2015 Andreas Merkle
// <vscp@blue-andi.de>
//
// Licence:
// The MIT License (MIT)
// [OSI Approved License]
//
// The MIT License (MIT)
//
// Copyright (c) 2012-2016 Paradise of the Frog/Grodans Paradis AB
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// Alternative licenses for VSCP & Friends may be arranged by contacting
// Grodans Paradis AB at info@grodansparadis.com, http://www.grodansparadis.com
//

/*jshint bitwise: false */

/**
 * This class implements the decision matrix NG.
 */
function dmNG() {
    // Contains all rules
    this.rules = [];

    // Event fields
    this.event = [
        "Undefined",
        "Class",
        "Type",
        "Originating address",
        "Hard coded",
        "Priority",
        "Zone",
        "Sub-zone",
        "Number of data",
        "Data 0",
        "Data 1",
        "Data 2",
        "Data 3",
        "Data 4",
        "Data 5",
        "Data 6",
        "Data x"
    ];

    // Basic operators
    this.basicOperators = [
        "Undefined",
        "=",
        "<",
        ">",
        "<=",
        ">=",
        "mask"
    ];

    // Logic operators
    this.logicOperators = [
        "-",
        "AND",
        "OR"
    ];
}

/**
 * This method inserts a rule at the given index.
 *
 * @param {number} index    - Index (0..)
 * @param {dmNGRule} rule   - Decision matrix NG rule
 */
dmNG.prototype.insertRule = function ( index, rule ) {
    if ( rule instanceof dmNGRule ) {
        this.rules.splice( index, 0, rule );
    }
    else {
        console.log( "Invalid type." );
    }
};

/**
 * This method deletes the rule at the given index.
 *
 * @param {number} index - Index (0..)
 */
dmNG.prototype.deleteRule = function ( index ) {
    this.rules.splice( index, 1 );
};

/**
 * This method returns the current rule set size in byte.
 *
 * @return {number} Rule set size in byte
 */
dmNG.prototype.getSize = function() {
    
    var size    = 1;
    var index   = 0;
    
    for( index = 0; index < this.rules.length; ++index ) {
    
        size += this.rules[ index ].getSize() + 1;
    }
    
    return size;
};

/**
 * This method returns the configuration of the decision matrix next generation.
 *
 * @return {number[]} Configuration as array
 */
dmNG.prototype.getConfig = function () {
    var config          = [];
    var ruleIndex       = 0;
    var conditionIndex  = 0;
    var operator        = 0;
    var ruleConfig      = [];

    // Number of rules
    config.push( this.rules.length );

    // For each rule ...
    for( ruleIndex = 0; ruleIndex < this.rules.length; ++ruleIndex ) {
        
        // Clear single rule configuration
        ruleConfig = [];

        // Write action id and action parameter
        ruleConfig.push( this.rules[ ruleIndex ].actionId );
        ruleConfig.push( this.rules[ ruleIndex ].actionPar );

        // For each condition ...
        for( conditionIndex = 0; conditionIndex < this.rules[ ruleIndex ].conditions.length; ++conditionIndex ) {
        
            // Build operator
            operator = ( ( this.rules[ ruleIndex ].conditions[ conditionIndex ].logicOperator & 0x0f ) << 4 ) |
                       ( ( this.rules[ ruleIndex ].conditions[ conditionIndex ].basicOperator & 0x0f ) << 0 );

            // Write operator
            ruleConfig.push( operator );
            
            // Write event parameter
            ruleConfig.push( this.rules[ ruleIndex ].conditions[ conditionIndex ].event );

            // Data x
            if ( 16 === this.rules[ ruleIndex ].conditions[ conditionIndex ].event ) {
                ruleConfig.push( this.rules[ ruleIndex ].conditions[ conditionIndex ].dataXIndex );
            }

            // Class
            if ( 1 === this.rules[ ruleIndex ].conditions[ conditionIndex ].event ) {
                ruleConfig.push( ( this.rules[ ruleIndex ].conditions[ conditionIndex ].value >> 8) & 0xff );
                ruleConfig.push( ( this.rules[ ruleIndex ].conditions[ conditionIndex ].value >> 0) & 0xff );
            }
            else {
                ruleConfig.push( this.rules[ ruleIndex ].conditions[ conditionIndex ].value & 0xff );
            }
        }

        config.push( ruleConfig.length + 1 );
        
        // Append rule configuration to rule set configuration
        config = config.concat( ruleConfig );
    }

    return config;
};

/**
 * This method sets the configuration of the decision matrix next generation.
 *
 * @param {number[]} config - Configuration
 */
dmNG.prototype.setConfig = function ( config ) {

    var index           = 1;
    var numRules        = config[ 0 ];
    var dmNGTmp         = new dmNG();
    var isError         = false;
    var byteCnt         = 0;
    var ruleIndex       = 0;
    var abort           = false;
    var ruleSize        = 0;
    var conditionIndex  = 0;
        
    if ( ( "undefined" === typeof config ) ||
         ( 0 === config.length ) ) {

        console.log( "Parameter error." );
        return;
    }

    // Determine configuration size in byte
    ++byteCnt;
    while( ( index < config.length ) && ( false === abort ) ) {
        
        if ( 0 === config[ index ] ) {
            abort = true;
        }
    
        byteCnt += config[ index ];
        index += config[ index ];
    }
    
    if ( true === abort ) {
        console.log( "Invalid configuration." );
        return;
    }

    // If the calculated configuration size is not equal than the configuration
    // array size, it will be stopped now.
    if ( byteCnt !== config.length ) {
        console.log( "Invalid configuration." );
        return;
    }

    // For each rule ...
    index = 1;
    for( ruleIndex = 0; ruleIndex < numRules; ++ruleIndex) {
        
        // Get rule size
        ruleSize = config[ index ] - 1;
        ++index;
        
        // Create a empty rule
        dmNGTmp.insertRule( ruleIndex, new dmNGRule() );

        // Set action id and action parameter
        dmNGTmp.rules[ ruleIndex ].actionId  = config[ index ];
        ++index;
        dmNGTmp.rules[ ruleIndex ].actionPar = config[ index ];
        ++index;
        
        ruleSize -= 2;

        // For each condition ...
        conditionIndex = 0;
        while( 0 < ruleSize ) {
           
            // Create a empty condition
            dmNGTmp.rules[ ruleIndex ].insertCondition( conditionIndex, new dmNGRuleCondition() );

            // Get basic and logic operator
            dmNGTmp.rules[ ruleIndex ].conditions[ conditionIndex ].logicOperator = ( config[ index ] >> 4 ) & 0x0f;
            dmNGTmp.rules[ ruleIndex ].conditions[ conditionIndex ].basicOperator = ( config[ index ] >> 0 ) & 0x0f;
            ++index;
            
            // Get event parameter
            dmNGTmp.rules[ ruleIndex ].conditions[ conditionIndex ].event = config[ index ];
            ++index;
            
            ruleSize -= 2;

            // Data x
            if ( 16 === dmNGTmp.rules[ ruleIndex ].conditions[ conditionIndex ].event ) {
                dmNGTmp.rules[ ruleIndex ].conditions[ conditionIndex ].dataXIndex = config[ index++ ];
                --ruleSize;
            }

            // Class
            if ( 1 === dmNGTmp.rules[ ruleIndex ].conditions[ conditionIndex ].event ) {
                dmNGTmp.rules[ ruleIndex ].conditions[ conditionIndex ].value  = config[ index++ ] << 8;
                dmNGTmp.rules[ ruleIndex ].conditions[ conditionIndex ].value |= config[ index++ ] << 0;
                ruleSize -= 2;
            }
            else {
                dmNGTmp.rules[ ruleIndex ].conditions[ conditionIndex ].value = config[ index++ ];
                --ruleSize;
            }

            ++conditionIndex;
        }
    }

    // If no error happened, set the configuration
    if ( false === isError ) {
        this.rules = dmNGTmp.rules;
    }
};

/**
 * This class implements a decision matrix NG rule.
 * @class
 */
function dmNGRule() {
    this.actionId = 0;
    this.actionPar = 0;
    this.conditions = [];
}

/**
 * This method inserts a condition at the given index.
 *
 * @param {number} index                - Index (0..)
 * @param {dmNGRuleCondition} condition - Decision matrix NG rule condition
 */
dmNGRule.prototype.insertCondition = function ( index, condition ) {
    if ( condition instanceof dmNGRuleCondition ) {
        this.conditions.splice( index, 0, condition );
    }
    else {
        console.log( "Invalid type." );
    }
};

/**
 * This method deletes the condition at the given index.
 *
 * @param {number} index - Index (0..)
 */
dmNGRule.prototype.deleteCondition = function ( index ) {
    this.conditions.splice( index, 1 );
};

/**
 * This method returns the size of the rule.
 *
 * @return {number} Rule size in byte
 */
dmNGRule.prototype.getSize = function() {
    
    var size    = 2;
    var index   = 0;
    
    for( index = 0; index < this.conditions.length; ++index ) {
        
        // Class
        if ( 1 === this.conditions[ index ].event ) {
            size += 4;
        }
        // Data X
        else if ( 16 === this.conditions[ index ].event ) {
            size += 4;
        }
        else {
            size += 3;
        }
    }
    
    return size;
};

/**
 * This class implements a decision matrix NG rule condition.
 * @class
 */
function dmNGRuleCondition() {
    this.event = 0;
    this.value = 0;
    this.basicOperator = 0;
    this.logicOperator = 0;
    this.dataXIndex = 0;
}
