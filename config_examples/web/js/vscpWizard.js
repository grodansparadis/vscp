// VSCP wizard javascript library
// Copyright (c) 2015 Andreas Merkle
// <vscp@blue-andi.de>
//
// Licence:
// The MIT License (MIT)
// [OSI Approved License]
//
// The MIT License (MIT)
//
// Copyright (c) 2012-2016 Grodans Paradis AB (Paradise of the Frog)
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

/* Create the root namespace and making sure we're not overwriting it */
var vscp = vscp || {};

/* ---------------------------------------------------------------------- */

/** VSCP wizard functionality based on a MDF
 * @namespace vscp.wizard
 */
vscp._createNS( "vscp.wizard" );

/**
 * A message box.
 * @class
 *
 * @param {object} options              - Options
 * @param {object} options.messageBox   - Messagebox as jquery xml object
 */
vscp.wizard.MessageBox = function( options ) {

    /**
     * Function input or output
     * @member {string}
     */
    this.func           = "input";
    /**
     * Head
     * @member {string}
     */
    this.head           = "";
    /**
     * Description
     * @member {string}
     */
    this.description    = "";
    /**
     * Variable type
     * @member {string}
     */
    this.variableType   = "";
    /**
     * Variable name
     * @member {string}
     */
    this.variableName   = "";
    /**
     * Variable value
     * @member {string}
     */
    this.variableValue  = "";

    if ( "undefined" !== typeof options ) {

        if ( "object" === typeof options.messageBox ) {

            this.parse( options.messageBox );
        }
    }
};

/**
 * Parse a messagebox object.
 *
 * @param {object} $messageBox - Messagebox as jquery xml object
 */
vscp.wizard.MessageBox.prototype.parse = function( $messageBox ) {

    if ( "undefined" === typeof $messageBox ) {
        return;
    }

    if ( "undefined" !== typeof $messageBox.children( "function" ) ) {
        this.func = $messageBox.children( "function" ).text();
    }
    
    if ( "undefined" !== typeof $messageBox.children( "head" ) ) {
        this.head = $messageBox.children( "head" ).text();
    }
    
    if ( "undefined" !== typeof $messageBox.children( "description" ) ) {
        this.description = $messageBox.children( "description" ).text();
    }
    
    if ( "undefined" !== typeof $messageBox.children( "variable" ) ) {
    
        if ( "undefined" !== typeof $messageBox.children( "variable" ).attr( "name" ) ) {
            this.variableName = $messageBox.children( "variable" ).attr( "name" );
        }
    
        if ( "undefined" !== typeof $messageBox.children( "variable" ).attr( "type" ) ) {
        
            this.variableType = $messageBox.children( "variable" ).attr( "type" );
        }
    }

};

/**
 * Bit in register access method.
 * @class
 *
 * @param {object} options                  - Options
 * @param {object} options.writeBitInReg    - write-bit-in-reg as jquery xml object
 */
vscp.wizard.WriteBitInReg = function( options ) {

    /** Bit position
     * @member {number}
     */
    this.pos            = 0;
    /** Register page
     * @member {number}
     */
    this.page           = 0;
    /** Register offset
     * @member {number}
     */
    this.offset         = 0;
    /** Bit width
     * @member {number}
     */
    this.width          = 1;
    /** Value of bit width
     * @member {number}
     */
    this.value          = 0;
    /** Variable name
     * @member {string}
     */
    this.variableName   = "";

    if ( "undefined" !== typeof options ) {

        if ( "object" === typeof options.writeBitInReg ) {

            this.parse( options.writeBitInReg );
        }
    }
};

/**
 * Parse a bit in register access method object.
 *
 * @param {object} writeBitInReg - write-bit-in-reg as jquery xml object
 */
vscp.wizard.WriteBitInReg.prototype.parse = function( $writeBitInReg ) {

    if ( "undefined" === typeof $writeBitInReg ) {
        return;
    }
    
    if ( "undefined" !== typeof $writeBitInReg.attr( "pos" ) ) {
        this.pos = parseInt( $writeBitInReg.attr( "pos" ) );
    }

    if ( "undefined" !== typeof $writeBitInReg.attr( "page" ) ) {
        this.page = parseInt( $writeBitInReg.attr( "page" ) );
    }
    
    if ( "undefined" !== typeof $writeBitInReg.attr( "offset" ) ) {
        this.offset = parseInt( $writeBitInReg.attr( "offset" ) );
    }
    
    if ( "undefined" !== typeof $writeBitInReg.attr( "width" ) ) {
        this.width = parseInt( $writeBitInReg.attr( "width" ) );
    }
    
    if ( "undefined" !== typeof $writeBitInReg.attr( "value" ) ) {

        // Does the value contains a variable?
        if ( "$" === $writeBitInReg.attr( "value" ).charAt( 0 ) ) {
            this.variableName = $writeBitInReg.attr( "value" ).substring( 1 );
        }
        // Does the value contains a boolean value?
        else if ( "false" === $writeBitInReg.attr( "value" ) ) {
            this.value = 0;
        }
        // Does the value contains a boolean value?
        else if ( "true" === $writeBitInReg.attr( "value" ) ) {
            this.value = 1;
        }
        // Value contains a number
        else {
            this.value = parseInt( $writeBitInReg.attr( "value" ) );
        }
    }

};

/**
 * Bit in abstraction access method.
 * @class
 *
 * @param {object} options                          - Options
 * @param {object} options.writeBitInAbstraction    - write-bit-in-abstraction as jquery xml object
 */
vscp.wizard.WriteBitInAbstraction = function( options ) {

    /** Abstract value id
     * @member {string}
     */
    this.id             = "";
    /** Bit position
     * @member {number}
     */
    this.pos            = 0;
    /** Bit width
     * @member {number}
     */
    this.width          = 1;
    /** Value of bit width
     * @member {number}
     */
    this.value          = 0;
    /** Variable name
     * @member {string}
     */
    this.variableName   = "";

    if ( "undefined" !== typeof options ) {

        if ( "object" === typeof options.writeBitInAbstraction ) {

            this.parse( options.writeBitInAbstraction );
        }
    }
};

/**
 * Parse a bit in abstraction access method object.
 *
 * @param {object} $writeBitInAbstraction - write-bit-in-abstraction as jquery xml object
 */
vscp.wizard.WriteBitInAbstraction.prototype.parse = function( $writeBitInAbstraction ) {

    if ( "undefined" === typeof $writeBitInAbstraction ) {
        return;
    }

    if ( "undefined" !== typeof $writeBitInAbstraction.attr( "id" ) ) {
        this.id = $writeBitInAbstraction.attr( "id" );
    }
    
    if ( "undefined" !== typeof $writeBitInAbstraction.attr( "pos" ) ) {
        this.pos = parseInt( $writeBitInAbstraction.attr( "pos" ) );
    }
    
    if ( "undefined" !== typeof $writeBitInAbstraction.attr( "width" ) ) {
        this.width = parseInt( $writeBitInAbstraction.attr( "width" ) );
    }
    
    if ( "undefined" !== typeof $writeBitInAbstraction.attr( "value" ) ) {

        // Does the value contains a variable?
        if ( "$" === $writeBitInAbstraction.attr( "value" ).charAt( 0 ) ) {
            this.variableName = $writeBitInAbstraction.attr( "value" ).substring( 1 );
        }
        // Does the value contains a boolean value?
        else if ( "false" === $writeBitInAbstraction.attr( "value" ) ) {
            this.value = 0;
        }
        // Does the value contains a boolean value?
        else if ( "true" === $writeBitInAbstraction.attr( "value" ) ) {
            this.value = 1;
        }
        // Value contains a number
        else {
            this.value = parseInt( $writeBitInAbstraction.attr( "value" ) );
        }
    }
    
};

/**
 * Register access method.
 * @class
 *
 * @param {object} options                  - Options
 * @param {object} options.writeRegister    - write-register as jquery xml object
 */
vscp.wizard.WriteRegister = function( options ) {

    /** Register page
     * @member {number}
     */
    this.page           = 0;
    /** Register offset
     * @member {number}
     */
    this.offset         = 0;
    /** Register value
     * @member {number}
     */
    this.value          = 0;
    /** Variable name
     * @member {string}
     */
    this.variableName   = "";

    if ( "undefined" !== typeof options ) {

        if ( "object" === typeof options.writeRegister ) {

            this.parse( options.writeRegister );
        }
    }
};

/**
 * Parse a register access method object.
 *
 * @param {object} $writeRegister - write-register as jquery xml object
 */
vscp.wizard.WriteRegister.prototype.parse = function( $writeRegister ) {

    if ( "undefined" === typeof $writeRegister ) {
        return;
    }
    
    if ( "undefined" !== typeof $writeRegister.attr( "page" ) ) {
        this.page = parseInt( $writeRegister.attr( "page" ) );
    }
    
    if ( "undefined" !== typeof $writeRegister.attr( "offset" ) ) {
        this.offset = parseInt( $writeRegister.attr( "offset" ) );
    }
    
    if ( "undefined" !== typeof $writeRegister.attr( "value" ) ) {

        // Does the value contains a variable?
        if ( "$" === $writeRegister.attr( "value" ).charAt( 0 ) ) {
            this.variableName = $writeRegister.attr( "value" ).substring( 1 );
        }
        // Value contains a number
        else {
            this.value = parseInt( $writeRegister.attr( "value" ) );
        }
    }

};

/**
 * Abstraction access method.
 * @class
 *
 * @param {object} options                  - Options
 * @param {object} options.writeAbstraction - write-abstraction as jquery xml object
 */
vscp.wizard.WriteAbstraction = function( options ) {

    /** Abstract value id
     * @member {string}
     */
    this.id             = "";
    /** Abstract value
     * @member {number}
     */
    this.value          = 0;
    /** Variable name
     * @member {number}
     */
    this.variableName   = "";

    if ( "undefined" !== typeof options ) {

        if ( "object" === typeof options.writeAbstraction ) {

            this.parse( options.writeAbstraction );
        }
    }
};

/**
 * Parse a abstraction access method object.
 *
 * @param {object} $writeAbstraction write-abstraction as jquery xml object
 */
vscp.wizard.WriteAbstraction.prototype.parse = function( $writeAbstraction ) {

    if ( "undefined" === typeof $writeAbstraction ) {
        return;
    }
    
    if ( "undefined" !== typeof $writeAbstraction.attr( "id" ) ) {
        this.id = $writeAbstraction.attr( "id" );
    }
    
    if ( "undefined" !== typeof $writeAbstraction.attr( "value" ) ) {

        // Does the value contains a variable?
        if ( "$" === $writeAbstraction.attr( "value" ).charAt( 0 ) ) {
            this.variableName = $writeAbstraction.attr( "value" ).substring( 1 );
        }
        // Value contains a number
        else {
            this.value = parseInt( $writeAbstraction.attr( "value" ) );
        }
    }
    
};

/**
 * A recipe.
 * @class
 *
 * @param {object} options          - Options
 * @param {object} options.recipe   - Recipe as jquery xml object
 * @param {object} options.mdf      - MDF as jquery xml object
 */
vscp.wizard.Recipe = function( options ) {

    /** Recipe name
     * @member {string}
     */
    this.name                   = "";
    /** Recipe description
     * @member {string}
     */
    this.description            = "";
    /** Bit access methods in registers
     * @member {vscp.wizard.WriteBitInReg[]}
     */
    this.writeBitInRegs         = [];
    /** Bit access methods in abstract value
     * @member {vscp.wizard.WriteBitAbstractions[]}
     */
    this.writeBitInAbstractions = [];
    /** Register access methods
     * @member {vscp.wizard.WriteRegister[]}
     */
    this.writeRegisters         = [];
    /** Abstract access methods
     * @member {vscp.wizard.WriteAbstraction[]}
     */
    this.writeAbstractions      = [];
    /** Messageboxes
     * @member {vscp.wizard.MessageBox[]}
     */
    this.messageBoxes           = [];
    /** MDF
     * @member {object}
     */
    this.mdf                    = null;

    if ( "undefined" !== typeof options ) {

        if ( "object" === typeof options.recipe ) {

            this.parse( options.recipe );
        }

        if ( "object" ===  typeof options.mdf ) {

            this.mdf = options.mdf;
        }
    }
};

/**
 * Parse a recipe object.
 *
 * @param {object} $recipe - Recipe as jquery xml object
 */
vscp.wizard.Recipe.prototype.parse = function( $recipe ) {

    var writeBitInRegs          = this.writeBitInRegs;
    var writeBitInReg           = null;
    var writeBitInAbstractions  = this.writeBitInAbstractions;
    var writeBitInAbstraction   = null;
    var writeRegisters          = this.writeRegisters;
    var writeRegister           = null;
    var writeAbstractions       = this.writeAbstractions;
    var writeAbstraction        = null;
    var messageBoxes            = this.messageBoxes;
    var messageBox              = null;

    if ( "undefined" === typeof $recipe ) {
        return;
    }

    if ( "undefined" !== typeof $recipe.children( "name" ) ) {
        this.name = $recipe.children( "name" ).text();
    }
    
    if ( "undefined" !== typeof $recipe.children( "description" ) ) {
        this.description = $recipe.children( "description" ).text();
    }
    
    $recipe.children( "write-bit-in-reg" ).each( function() {
        
        writeBitInReg = new vscp.wizard.WriteBitInReg({
            writeBitInReg: $( this )
        });

        writeBitInRegs.push( writeBitInReg );
        
    });

    $recipe.children( "write-bit-in-abstraction" ).each( function() {
        
        writeBitInAbstraction = new vscp.wizard.WriteBitInAbstraction({
            writeBitInAbstraction: $( this )
        });

        writeBitInAbstractions.push( writeBitInAbstraction );
        
    });

    $recipe.children( "write-register" ).each( function() {
        
        writeRegister = new vscp.wizard.WriteRegister({
            writeRegister: $( this )
        });

        writeRegisters.push( writeRegister );
        
    });
    
    $recipe.children( "write-abstraction" ).each( function() {
        
        writeAbstraction = new vscp.wizard.WriteAbstraction({
            writeAbstraction: $( this )
        });

        writeAbstractions.push( writeAbstraction );
        
    });

    $recipe.children( "messagebox" ).each( function() {
        
        messageBox = new vscp.wizard.MessageBox({
            messageBox: $( this )
        });

        messageBoxes.push( messageBox );
        
    });
    
};

/**
 * Write a recipe.
 *
 * @param {object] options                      - Options
 * @param {vscp.Connection} options.connection  - VSCP connection
 * @param {number] options.nodeId               - Node id
 */
vscp.wizard.Recipe.prototype.write = function( options ) {

    var writeBitInRegsCnt           = 0;
    var writeBitInAbstractionsCnt   = 0;
    var writeRegistersCnt           = 0;
    var writeAbstractionsCnt        = 0;
    var onError                     = null;
    var messageBoxIndex             = 0;
    var index                       = 0;
    var onProgress                  = null;
    var steps                       = 0;
    var progress                    = 0;

    var updateProgress = function( percent ) {
        if ( null !== onProgress ) {
            onProgress( percent );
        }
    };
    
    var process = function() {

        progress += Math.floor( 100 / steps );
        updateProgress( progress );
    
        if ( this.writeBitInRegs.length > writeBitInRegsCnt ) {
            console.debug( vscp.utility.getTime() + " Recipe " + this.name + ": Write bit in register value." );
            
            vscp.writeRegister.writeBits({

                connection: options.connection,

                nodeId: options.nodeId,

                page: this.writeBitInRegs[ writeBitInRegsCnt ].page,

                offset: this.writeBitInRegs[ writeBitInRegsCnt ].offset,

                pos: this.writeBitInRegs[ writeBitInRegsCnt ].pos,

                width: this.writeBitInRegs[ writeBitInRegsCnt ].width,

                value: this.writeBitInRegs[ writeBitInRegsCnt ].value,

                onSuccess: process,

                onError: onError
            });

            ++writeBitInRegsCnt;
        }
        else if ( this.writeBitInAbstractions.length > writeBitInAbstractionsCnt ) {
            console.debug( vscp.utility.getTime() + " Recipe " + this.name + ": Write bit in abstract value." );

            vscp.mdf.writeAbstractBits({

                connection: options.connection,

                nodeId: options.nodeId,

                mdf: this.mdf,

                id: this.writeBitInAbstractions[ writeBitInAbstractionsCnt ].id,

                pos: this.writeBitInAbstractions[ writeBitInAbstractionsCnt ].pos,

                width: this.writeBitInAbstractions[ writeBitInAbstractionsCnt ].width,

                value: this.writeBitInAbstractions[ writeBitInAbstractionsCnt ].value,

                onSuccess: process,

                onError: onError

            });

            ++writeBitInAbstractionsCnt;
        }
        else if ( this.writeRegisters.length > writeRegistersCnt ) {
            console.debug( vscp.utility.getTime() + " Recipe " + this.name + ": Write register value." );

            vscp.register.write({

                connection: options.connection,

                nodeId: options.nodeId,

                page: this.writeRegisters[ writeRegistersCnt ].page,

                offset: this.writeRegisters[ writeRegistersCnt ].offset,

                count: 1,
                
                data: [ this.writeRegisters[ writeRegistersCnt ].value ],

                onSuccess: process,

                onError: onError

            });

            ++writeRegistersCnt;
        }
        else if ( this.writeAbstractions.length > writeAbstractionsCnt ) {
            console.debug( vscp.utility.getTime() + " Recipe " + this.name + ": Write abstract value." );

            vscp.mdf.writeAbstractValue({

                connection: options.connection,

                nodeId: options.nodeId,

                mdf: this.mdf,

                id: this.writeAbstractions[ writeAbstractionsCnt ].id,

                value: this.writeAbstractions[ writeAbstractionsCnt ].value,

                onSuccess: process,

                onError: onError

            });

            ++writeAbstractionsCnt;
        }
        else {
            console.info( "Recipe written." );

            options.onSuccess();
        }

    }.bind( this );
    
    if ( "undefined" === typeof options ) {
        console.error( vscp.utility.getTime() + " Options are missing." );
        return;
    }
    
    if ( false === ( options.connection instanceof vscp.Connection ) ) {
        console.error( vscp.utility.getTime() + " VSCP connection object is missing." );
        return;
    }
    
    if ( "number" !== typeof options.nodeId ) {
        console.error( vscp.utility.getTime() + " Node id is missing." );
        return;
    }
    
    if ( "function" === typeof options.onProgress ) {
        onProgress = options.onProgress;
    }
    
    if ( "function" !== typeof options.onSuccess ) {
        console.error( vscp.utility.getTime() + " onSuccess callback is missing." );
        return;
    }
    
    if ( "function" === typeof options.onError ) {
        onError = options.onError;
    }
    
    // Get all messagebox variables and set the variable value to the access method objects
    for( messageBoxIndex = 0; messageBoxIndex < this.messageBoxes.length; ++messageBoxIndex ) {

        for( index = 0; index < this.writeBitInRegs.length; ++index ) {
            if ( this.messageBoxes[ messageBoxIndex ].variableName === this.writeBitInRegs[ index ].variableName ) {
                this.writeBitInRegs[ index ].value = this.messageBoxes[ messageBoxIndex ].variableValue;
            }
        }
        
        for( index = 0; index < this.writeBitInAbstractions.length; ++index ) {
            if ( this.messageBoxes[ messageBoxIndex ].variableName === this.writeBitInAbstractions[ index ].variableName ) {
                this.writeBitInAbstractions[ index ].value = this.messageBoxes[ messageBoxIndex ].variableValue;
            }
        }

        for( index = 0; index < this.writeRegisters.length; ++index ) {
            if ( this.messageBoxes[ messageBoxIndex ].variableName === this.writeRegisters[ index ].variableName ) {
                this.writeRegisters[ index ].value = this.messageBoxes[ messageBoxIndex ].variableValue;
            }
        }
        
        for( index = 0; index < this.writeAbstractions.length; ++index ) {
            if ( this.messageBoxes[ messageBoxIndex ].variableName === this.writeAbstractions[ index ].variableName ) {
                this.writeAbstractions[ index ].value = this.messageBoxes[ messageBoxIndex ].variableValue;
            }
        }
    }
    
    // How many steps are necessary to write the whole recipe?
    steps += this.writeBitInRegs.length;
    steps += this.writeBitInAbstractions.length;
    steps += this.writeRegisters.length;
    steps += this.writeAbstractions.length;
    steps += 1;
    
    // Start writing the recipe
    console.info( vscp.utility.getTime() + " Writing recipe " + this.name + "." );
    process();
};

/**
 * Get recipes from a MDF in JSON format.
 *
 * @param {object] options      - Options
 * @param {object} options.mdf  - The mdf as jquery xml object
 *
 * @return {vscp.wizard.Recipe[]} Recipe array
 */
vscp.wizard.getRecipes = function( options ) {

    var recipes = [];
    var recipe  = null;

    if ( "object" !== typeof options.mdf ) {
        console.error( vscp.utility.getTime() + " MDF object is missing." );
        return recipes;
    }
    
    // Get all recipes
    options.mdf.find( "vscp > module > setup > recipe" ).each( function() {
        
        recipe = new vscp.wizard.Recipe({
            recipe: $( this ),
            mdf: options.mdf
        });
        
        recipes.push( recipe );

    });
    
    return recipes;
};

