//<debug>
Ext.Loader.setPath({
    'Ext': './'
});
//</debug>

// Define the model
Ext.define('ListItem', {
    extend: 'Ext.data.Model',
    config: {
        fields: [
			{name: 'id', 		type: 'string'},
            {name: 'label', 	type: 'string'},
			{name: 'type', 		type: 'string'},
			{name: 'icon', 		type: 'string'}
        ]
    }
});


Ext.application({
    glossOnIcon: false,
    icon: {
        57: './resources/icons/icon.png',
        72: './resources/icons/icon@72.png',
        114: './resources/icons/icon@2x.png',
        144: './resources/icons/icon@114.png'
    },

    phoneStartupScreen: './resources/loading/Homescreen.jpg',
    tabletStartupScreen: './resources/loading/Homescreen~ipad.jpg',

    requires: [
        'Ext.data.TreeStore',
        'Ext.NestedList',
        'Ext.TitleBar'
    ],

    launch: function() {
	
		// Create the treestore
        var store = Ext.create('Ext.data.TreeStore', {
            model: 'ListItem',
            autoLoad: true,
			sorters: 'type',
			grouped: true,
			grouper: function(record) {
				return record.get('label');
			},
            proxy: {
                type: 'ajax',
                url: 'source.json'
            }
        });

		// Create the nested list
        Ext.create('Ext.NestedList', {
		
            fullscreen: true,
            title: 'VSCP-Simple UI',
            displayField: 'label',		
			cls: 'Special',
			
			// Format Title
            getTitleTextTpl: function() {
                return '{' + 
						this.getDisplayField() + 
						'}<tpl if="leaf !== true">*</tpl>';
            },
			
            // Format Items
            getItemTextTpl: function() {
                return (	
					'<tpl if="leaf !== true">'
					+ '<div class="itemleft">'
					+ '<img align="top" src="resources/images/house.png" width="32">'
					+ '</div>'
					+ '</tpl>'
					+ '<tpl if="leaf === true">'
					+ '<div class="itemleft">'
					+ '<img align="top" src="resources/images/arrow_right.png" width="32">'
					+ '</div>'
					+ '</tpl>'
					+ '<div class="itemgroup">{ '
					+ this.getDisplayField() + 
					'}</div>');
            },
			
			getSubList: function (node) {
			
				var items  = this.items,
								list,
								itemId = node.internalId;

				alert('Test');
				
				// can be invoked prior to items being transformed into
				// a MixedCollection
				if (items && items.get) {
					list = items.get(itemId);
				}

				if (list) {
					//add support for grouping
					list.store.getGroupString = this.store.getGroupString;
					list.grouped = this.grouped;
					list.indexBar = this.indexBar;
					return list;
				} 
				else {
					var config = this.getListConfig(node);
					//add support for grouping
					config.store.getGroupString = this.store.getGroupString;
					config.grouped = this.grouped;
					config.indexBar = this.indexBar;
					return config;
				}
			},
			
            // provide a codebox for each source file
            store: store,
            listeners: {
				
				// Nested list
                leafitemtap: function( me, list, index, item, e ) {
				
                    var store = list.getStore(),
                        record  = store.getAt(index),
                        detailCard = me.getDetailCard();

                    list.setMasked({
						xtype: 'loadmask',
						message: 'Loading'
                    });

                    Ext.Ajax.request({
                        url: './' + record.get('id'),
                        success: function(response) {
                            detailCard.setHtml(response.responseText);
                            list.unmask();
                        },
                        failure: function() {
                            detailCard.setHtml("Loading failed.");
                            list.unmask();
                        }
                    });

                }
            }
        });
    }
});
