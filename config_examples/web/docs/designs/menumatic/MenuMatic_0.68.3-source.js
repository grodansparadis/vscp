
/**
 * menuMatic 
 * @version 0.68.3 (beta)
 * @author Jason J. Jaeger | greengeckodesign.com
 * @copyright 2008 Jason John Jaeger
 * @license MIT-style License
 *			Permission is hereby granted, free of charge, to any person obtaining a copy
 *			of this software and associated documentation files (the "Software"), to deal
 *			in the Software without restriction, including without limitation the rights
 *			to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *			copies of the Software, and to permit persons to whom the Software is
 *			furnished to do so, subject to the following conditions:
 *	
 *			The above copyright notice and this permission notice shall be included in
 *			all copies or substantial portions of the Software.
 *	
 *			THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *			IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *			FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *			AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *			LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *			OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 *			THE SOFTWARE.
 *	
 *  @changeLog_________________________________________________________________________________
 *  
 *  Nov  15th 2008
 *  JJJ - incremented version to 0.68.3
 *  	- Fixed bug which occured when ol was used instead of ul.. was because you cannot use selectors such 
 *  		as 'ul, ol' with getParent() and getNext() despite the mootool documentation saying "if Selectors 
 *  		is included, a full CSS selector can be passed."
 *  
 *  Nov 4th 2008
 *  JJJ - incremented version to 0.68.2
 *  	- Removed extranious comma which threw an error in IE. (Thanks to Robert S. for catching that)
 *  	- Added check to change the opacity to 99.9 if it is over 99 due to the fact that 
 *  		setting the opaicty to 100% in safari caused misc strange behaviour (thanks to Roland Barker 
 *  		at xnau webdesign for reporting this one)
 *  
 *  Nov 3rd 2008
 *  JJJ - Renamed menuWidth option to stretchMainMenu and made it work much quicker
 *  
 *  Nov 2nd 2008
 *  JJJ - Finished adding several callback functions
 *  	- Added fixHasLayoutBug option
 *  
 *  Oct 4th 2008
 *  JJJ - Continued to go through code to optimize and clean it up
 *  	- Added center option
 *  
 *  Sept 28th 2008
 *  JJJ - Incremented version to 0.68
 *  	- Began optimizing & better commenting code
 *  
 *  Sept 16th 2008
 *  JJJ - Incremented version to 0.67
 *  	- Specifically set each submenu z-index on show, incrementing each time. Set z-index back to 1 on hide.
 *  
 *  Sept 14th 2008
 *  JJJ - Added bottom edge detection and redirection (if bottom of subsequent submenu is lower than the scrollheight of the body)
 *  
 *  Sept 10th 2008
 *  JJJ - Did a google search for MooMenu and it looks like that name is already being used, so I must reluctently change the name.
 *  		The name I have chose to move forward with is: MenuMatic.
 *  
 *  Sept 8th 2008
 *  JJJ - Incremented version to 0.66
 *  	- Fixed IE bug whiched caused some events to be incorrectly attached to parent buttons causing sub submenus to not open
 *  	  correctly when transitions are enabled. This error was introduced in 0.65. 
 *  
 *  August 10th 2008
 *  JJJ - Incremented verion to 0.65
 *  	- Fixed bug with edge detection and redirection
 *  	- Improved javascript and CSS to assure the submenus will always be on top
 *  	- Improved edge detection and redirection logic
 *  	- Added subMenusContainerId option so that if you have multiple MooMenu instances on a page they can be styled differently
 *  
 *  May 7th 2008
 *  JJJ - Incremented version to 0.64
 *  	- Fixed bug with that occured when firing events
 *  
 *  April 28th 2008:
 *  JJJ - Incremented version to 0.63
 *  	- Added killDivider option
 *  	- Removed focus on hover since it adds browser default dotted outline
 *  	- Changed menuWidth option to use actual width of nav element (so it can be set in css) instead of a width set in javascript
 *  
 *  Feb 17th 2008:
 *  JJJ - Incremented version to 0.62
 *  	- Added mmbClassName and mmbFocusedClassName options to allow morphing of main menu btns
 *  
 *  Feb 16th 2008:
 *  JJJ - Incremented version to 0.61
 *  	- Made changes to options syntax. 
 *  	- Improved keyboard accessability to take into account a menu's orientation, and its x and y direction when using arrow keys
 *  	- Added matchWidthMode option.
 *  
 *  Feb 14th 2008:
 *  JJJ - Incremented version to 0.60
 *  	- Added menuWidth option to simulate a table based layout for the main menu when the orienation is horizontal
 **/


/*
 * To Do:
 * 
 * 1. Make screencast tutorials
 * 
 * 2. Add more examples to website
 * 
 * 3. Clean up/optimize CSS / make easier to edit 
 * 
 * 4. Make online generator 
 * 
 * 5. Add rounded corners option
 * 
 * 6. Add drop shadow option
 * 
 * 7. Add sound integration 
 * 
 */

var MenuMatic = new Class({
	Implements: Options,
	options: {
        id: 'nav',//the id of the main menu (ul or ol)
        subMenusContainerId:'subMenusContainer',//id of the container div that will be generated to hold the submenus 
		
		//subMenu behavior
		effect: 'slide & fade',// 'slide', 'fade', 'slide & fade', or  null
		duration: 600,//duration of the effect in milliseconds
		physics: Fx.Transitions.Pow.easeOut,//how the effect behaves
		hideDelay: 1000,//in milliseconds, how long you have after moving your mouse off of the submenus before they dissapear
		
		//layout
		stretchMainMenu:false,//stretch main menu btn widths to fit within the width {set in the css} of the parent UL or OL
		matchWidthMode:false,//initial submenus match their parent button's width
		orientation: 'horizontal',//horizontal or vertical
		direction:{	x: 'right',	y: 'down' },//for submenus ( relative to the parent button )left or right, up or down
		tweakInitial:{ x:0, y:0	},//if you need to tweak the placement of the initial submenus
		tweakSubsequent:{ x:0, y:0 },//if you need to tweak the placement of the subsequent submenus
		center: false,// will attempt to center main nav element
		
		//dynamic style
		opacity: 95,//of the submenus
		mmbFocusedClassName:null,//main menu button classname, used for morphing to focused state
		mmbClassName:null,//main menu button classname, used for morphing back to original state
		killDivider:null,	
		
		fixHasLayoutBug:false,	
		
		onHideAllSubMenusNow_begin: (function(){}),
		onHideAllSubMenusNow_complete: (function(){}),
		
		onInit_begin: (function(){}),
		onInit_complete: (function(){})		

    },
	
	hideAllMenusTimeout:null,
	allSubMenus:[],
	subMenuZindex:1,
	
	initialize: function(options){		
		//if(Browser.Engine.webkit419){return;}		
        this.setOptions(options);
		this.options.onInit_begin();
		if(this.options.opacity > 99){this.options.opacity = 99.9;}
		this.options.opacity = this.options.opacity /100;

		Element.implement({
		    getId: function(){
				//If this element does not have an id, give it a unique one
		        if(!this.id){ 
					var uniqueId = this.get('tag') + "-" + $time();
					while($(uniqueId)){
						//make sure it is absolutely unique
						uniqueId = this.get('tag') + "-" + $time();						
					}
					this.id = uniqueId;						
				}
			    return this.id;
		    }
		});
		
		//initialize directions
		this.options.direction.x = this.options.direction.x.toLowerCase();
		this.options.direction.y = this.options.direction.y.toLowerCase();
		if(this.options.direction.x === 'right'){
			this.options.direction.xInverse = 'left';
		}else if(this.options.direction.x === 'left'){
			this.options.direction.xInverse = 'right';
		}
		if(this.options.direction.y === 'up'){
			this.options.direction.yInverse = 'down';
		}else if(this.options.direction.y === 'down'){
			this.options.direction.yInverse = 'up';
		}
		
		var links = $(this.options.id).getElements('a');
		
		links.each(function(item,index){
			
			//store parent links & child menu info
			item.store('parentLinks', item.getParent().getParents('li').getFirst('a'));
			
			item.store('parentLinks',item.retrieve('parentLinks').erase(item.retrieve('parentLinks').getFirst()));
			item.store('childMenu',item.getNext('ul') || item.getNext('ol'));
			
			//determine submenu type
			theSubMenuType = 'subsequent';
			
			//console.log($(this.options.id).getElements('ul, ol'));
			//console.log(item.getParent(['body ul,ol']));
			
			//console.log($(item.getParent('ul') || item.getParent('ol') ));
			
			if( $(item.getParent('ul') || item.getParent('ol') ).id === this.options.id){theSubMenuType = 'initial';	}
			item.store('subMenuType',theSubMenuType );
			
			//add classes to parents
			if(theSubMenuType === 'initial' && $(item.getNext('ul') || item.getNext('ol') )){
				item.addClass('mainMenuParentBtn');
			}else if($(item.getNext('ul') || item.getNext('ol') )){
				item.addClass('subMenuParentBtn');
			}			
		}.bind(this));
		
		//rip the submenus apart into separate divs inside of subMenusContainer
		var subMenusContainer = new Element('div', { 'id': this.options.subMenusContainerId	}).inject( $(document.body) ,'bottom');
		$(this.options.id).getElements('ul, ol').each(function(item,index){	
			new Element('div',{'class': 'smOW'}).inject(subMenusContainer).grab(item);		
		}.bind(this));		
		
		//set tabindex to -1 so tabbing through links in page does not go through hidden links in submenus container, since arrow keys can be used to navigate through submenus
		subMenusContainer.getElements('a').set('tabindex','-1'); 
		
		links.each(function(item,index){
			//only apply to links with subMenus
			if (!item.retrieve('childMenu')) {return;}
			
			//update childMenu pointer to look at smOW DIVs
			item.store('childMenu', item.retrieve('childMenu').getParent('div'));
			
			//add to allSubMenus array
			this.allSubMenus.include(item.retrieve('childMenu'));			
			
			//store parentSubMenus
			item.store('parentSubMenus',item.retrieve('parentLinks').retrieve('childMenu'));

			//now create the MenuMaticSubMenu class instances 
			var aSubMenu = new MenuMaticSubMenu(this.options,this, item);

		}.bind(this));
			
		//attach event handlers to non-parent main menu buttons
		var nonParentBtns = $(this.options.id).getElements('a').filter(function(item, index){ return !item.retrieve('childMenu'); });
		nonParentBtns.each(function(item, index){
			item.addEvents({
				'mouseenter': function(e){					
					//e = new Event(e).stop();
					this.hideAllSubMenusNow();	
					if(this.options.mmbClassName && this.options.mmbFocusedClassName){
						$(item).retrieve('btnMorph', new Fx.Morph(item, { 'duration':(this.options.duration/2), transition:this.options.physics, link:'cancel'})).start(this.options.mmbFocusedClassName); 
					}								
				}.bind(this),
				
				'focus': function(e){
					//e = new Event(e).stop();
					this.hideAllSubMenusNow();	
					if(this.options.mmbClassName && this.options.mmbFocusedClassName){
						$(item).retrieve('btnMorph', new Fx.Morph(item, { 'duration':(this.options.duration/2), transition:this.options.physics, link:'cancel'})).start(this.options.mmbFocusedClassName); 
					}
				}.bind(this),
				
				'mouseleave':function(e){
					//e = new Event(e).stop();
					if (this.options.mmbClassName && this.options.mmbFocusedClassName) {						
						$(item).retrieve('btnMorph', new Fx.Morph(item, {	'duration': (this.options.duration * 5),transition: this.options.physics,link: 'cancel'	})).start(this.options.mmbClassName);
					}	
				}.bind(this),
				
				'blur':function(e){
					//e = new Event(e).stop();
					if (this.options.mmbClassName && this.options.mmbFocusedClassName) {						
						$(item).retrieve('btnMorph', new Fx.Morph(item, {	'duration': (this.options.duration * 5),transition: this.options.physics,link: 'cancel'	})).start(this.options.mmbClassName);
					}					
				}.bind(this),
				
				'keydown' : function(e){
				    var event = new Event(e);
					if (e.key === 'up' || e.key === 'down' || e.key === 'left' || e.key === 'right') {	e.stop();	}
					
					if( e.key === 'left' && this.options.orientation === 'horizontal' || 
						e.key === 'up' && this.options.orientation === 'vertical'){
						
						if(item.getParent('li').getPrevious('li')){
							item.getParent('li').getPrevious('li').getFirst('a').focus();
						}else{
							item.getParent('li').getParent().getLast('li').getFirst('a').focus();
						}
					}else if(e.key === 'right' && this.options.orientation === 'horizontal' || 
							 e.key === 'down' && this.options.orientation === 'vertical'){
						if(item.getParent('li').getNext('li')){
							item.getParent('li').getNext('li').getFirst('a').focus();
						}else{
							item.getParent('li').getParent().getFirst('li').getFirst('a').focus();
						}	
					}
				}.bind(this)
			});
		}, this);

		
		this.stretch();
		this.killDivider();
		this.center();
		this.fixHasLayoutBug();
		this.options.onInit_complete();		
    },
	
	fixHasLayoutBug:function(){
		if(Browser.Engine.trident && this.options.fixHasLayoutBug){
			$(this.options.id).getParents().setStyle('zoom',1);
			$(this.options.id).setStyle('zoom',1);
			$(this.options.id).getChildren().setStyle('zoom',1);
			$(this.options.subMenusContainerId).setStyle('zoom',1);
			$(this.options.subMenusContainerId).getChildren().setStyle('zoom',1);
		}		
	},
	
	center:function(){
		if(!this.options.center){return;}
		$(this.options.id).setStyles({'left':'50%','margin-left': -($(this.options.id).getSize().x/2) });
	},
	
	stretch:function(){
		//stretch main menu btn widths to fit within the width of the parent UL or OL
		if(this.options.stretchMainMenu  && this.options.orientation === 'horizontal'){
			var targetWidth = parseFloat($(this.options.id).getCoordinates().width) ;
			var totalBtnWidth = 0;
			var mainBtns = $(this.options.id).getElements('a');
			mainBtns.setStyles({'padding-left':0,'padding-right':0});
			mainBtns.each(function(item,index){ totalBtnWidth+= item.getSize().x; }.bind(this));
			if(targetWidth < totalBtnWidth){return;}
			var increment = (targetWidth - totalBtnWidth)/ mainBtns.length;
			mainBtns.each(function(item,index){ item.setStyle('width',item.getSize().x+increment);	}.bind(this));
			mainBtns.getLast().setStyle('width',mainBtns.getLast().getSize().x-1);
		}
	},
	
	killDivider:function(){
		if(this.options.killDivider && this.options.killDivider.toLowerCase() === 'first'){
			$($(this.options.id).getElements('li')[0]).setStyles({'background':'none'});
		}else if(this.options.killDivider && this.options.killDivider.toLowerCase() === 'last'){
			$($(this.options.id).getElements('li').getLast()).setStyles({'background':'none'});
		}
	},

	hideAllSubMenusNow: function(){
		this.options.onHideAllSubMenusNow_begin();
		$clear(this.hideAllMenusTimeout);
		$$(this.allSubMenus).fireEvent('hide');
		this.options.onHideAllSubMenusNow_complete();	
	} 
	
});

var MenuMaticSubMenu = new Class({
	Implements: Options,
	Extends: MenuMatic,
    options: {
		onSubMenuInit_begin: (function(subMenuClass){}),
		onSubMenuInit_complete: (function(subMenuClass){}),
		
		onMatchWidth_begin: (function(subMenuClass){}),
		onMatchWidth_complete: (function(subMenuClass){}),
		
		onHideSubMenu_begin: (function(subMenuClass){}),
		onHideSubMenu_complete: (function(subMenuClass){}),
		
		onHideOtherSubMenus_begin: (function(subMenuClass){}),
		onHideOtherSubMenus_complete: (function(subMenuClass){}),		
		
		onHideAllSubMenus_begin: (function(subMenuClass){}),
		onHideAllSubMenus_complete: (function(subMenuClass){}),
		
		onPositionSubMenu_begin: (function(subMenuClass){}),
		onPositionSubMenu_complete: (function(subMenuClass){}),
		
		onShowSubMenu_begin: (function(subMenuClass){}),
		onShowSubMenu_complete: (function(subMenuClass){})
	},
	root:null,
	btn:null,
	hidden:true,
	myEffect:null,
		
	initialize: function(options,root,btn){
		this.setOptions(options);		
		this.root = root;
		this.btn = btn;
		this.childMenu = this.btn.retrieve('childMenu');
		this.subMenuType = this.btn.retrieve('subMenuType');
		this.childMenu = this.btn.retrieve('childMenu');
		this.parentSubMenus =  $$(this.btn.retrieve('parentSubMenus'));
		this.parentLinks =  $$(this.btn.retrieve('parentLinks'));
		this.parentSubMenu = $(this.parentSubMenus[0]);
		if(this.parentSubMenu ){this.parentSubMenu =this.parentSubMenu.retrieve('class');}
		this.childMenu.store('class',this);
		this.btn.store('class',this);
		this.childMenu.store('status','closed')
		
		this.options.onSubMenuInit_begin(this);		
		
		//add hide Event
		this.childMenu.addEvent('hide',function(){this.hideSubMenu();}.bind(this));
		
		//add show Event
		this.childMenu.addEvent('show',function(){this.showSubMenu();}.bind(this));

		if(this.options.effect){
			this.myEffect = new Fx.Morph(
				$(this.childMenu).getFirst(), {	duration: this.options.duration, transition: this.options.physics,  link: 'cancel' } 
			);
		}
		if(this.options.effect === 'slide' || this.options.effect === 'slide & fade'){
			if (this.subMenuType == 'initial' && this.options.orientation === 'horizontal' ) {
				this.childMenu.getFirst().setStyle('margin-top','0' );
			}else {
				this.childMenu.getFirst().setStyle('margin-left', '0');
			}
			
		}else if (this.options.effect === 'fade' || this.options.effect === 'slide & fade'){
			this.childMenu.getFirst().setStyle('opacity',0 );
		}
		
		if (this.options.effect != 'fade' && this.options.effect != 'slide & fade') {
			this.childMenu.getFirst().setStyle('opacity',this.options.opacity);
		}

		
		//attach event handlers to non-parent sub menu buttons
		var nonParentBtns = $(this.childMenu).getElements('a').filter(function(item, index){ return !item.retrieve('childMenu'); });
		nonParentBtns.each(function(item, index){
			$(item).addClass('subMenuBtn');
			
			item.addEvents({
				'mouseenter': function(e){
					this.childMenu.fireEvent('show');
					this.cancellHideAllSubMenus();					
					this.hideOtherSubMenus();				
				}.bind(this),
				
				'focus': function(e){
					this.childMenu.fireEvent('show');
					this.cancellHideAllSubMenus();		
					this.hideOtherSubMenus();
				}.bind(this),
				
				'mouseleave': function(e){
					this.cancellHideAllSubMenus();
					this.hideAllSubMenus();					
				}.bind(this),
				
				'blur': function(e){
					this.cancellHideAllSubMenus();
					this.hideAllSubMenus();
				}.bind(this),
				
				'keydown' : function(e){
				    var event = new Event(e);
					
					if (e.key === 'up' || e.key === 'down' || e.key === 'left' || e.key === 'right' || e.key === 'tab') {	e.stop();	}
					
					if(e.key === 'up'){
						if(item.getParent('li').getPrevious('li')){
							//move focus to the next link up if possible
							item.getParent('li').getPrevious('li').getFirst('a').focus();
						}else if(this.options.direction.y ==='down'){
							//move focus to the parent link
							this.btn.focus();
						}else if(this.options.direction.y ==='up'){
							//move focus to the last link in the subMenu
							item.getParent('li').getParent().getLast('li').getFirst('a').focus();
						}
					}else if(e.key === 'down'){
						if(item.getParent('li').getNext('li')){
							//move focus to the next link down if possible
							item.getParent('li').getNext('li').getFirst('a').focus();
						}else if(this.options.direction.y ==='down'){
							//move focus to the first link in the submenu
							item.getParent('li').getParent().getFirst('li').getFirst('a').focus();
						}else if(this.options.direction.y ==='up'){
							//move focus to the parent link
							this.btn.focus();
						}
					}else if(e.key === this.options.direction.xInverse){
						this.btn.focus();
					}
				}.bind(this)
			});
			
		}, this);
		
		$(this.btn).removeClass('subMenuBtn');
		
		if (this.subMenuType == 'initial') {
			this.btn.addClass('mainParentBtn');	
		}else{	
			this.btn.addClass('subParentBtn');	
		}
		
		//attach event handlers to parent button
		$(this.btn).addEvents({
			'mouseenter' : function(e){
				//e = new Event(e).stop();
				this.cancellHideAllSubMenus();
				this.hideOtherSubMenus();
				this.showSubMenu();
				if(this.subMenuType === 'initial' && this.options.mmbClassName && this.options.mmbFocusedClassName){
					$(this.btn).retrieve('btnMorph', new Fx.Morph($(this.btn), { 'duration':(this.options.duration/2), transition:this.options.physics, link:'cancel' })).start(this.options.mmbFocusedClassName);
				}
			}.bind(this),
			
			'focus' : function(e){
				//e = new Event(e).stop();
				this.cancellHideAllSubMenus();
				this.hideOtherSubMenus();
				this.showSubMenu();
				if(this.subMenuType === 'initial' && this.options.mmbClassName && this.options.mmbFocusedClassName){
					$(this.btn).retrieve('btnMorph', new Fx.Morph($(this.btn), { 'duration':(this.options.duration/2), transition:this.options.physics, link:'cancel' })).start(this.options.mmbFocusedClassName);
				}
			}.bind(this),
				
			'mouseleave': function(e){
				//e = new Event(e).stop();
				this.cancellHideAllSubMenus();
				this.hideAllSubMenus();
			}.bind(this),
			
			'blur': function(e){
				//e = new Event(e).stop();
				this.cancellHideAllSubMenus();
				this.hideAllSubMenus();
			}.bind(this),
			
			'keydown' : function(e){
			    e = new Event(e)
				if (e.key === 'up' || e.key === 'down' || e.key === 'left' || e.key === 'right') {	e.stop();	}
				
				if(!this.parentSubMenu){
					//main menu parent buttons
					if(
						this.options.orientation === 'horizontal' && e.key === this.options.direction.y ||
						this.options.orientation === 'vertical' && e.key === this.options.direction.x
					){
						if(this.options.direction.y ==='down'){
							//move focus to the first link in the child menu
							this.childMenu.getFirst().getFirst('li').getFirst('a').focus();
						}else if(this.options.direction.y ==='up'){
							//move focus to the first link in the child menu
							this.childMenu.getFirst().getLast('li').getFirst('a').focus();
						}
					}else if(
						this.options.orientation === 'horizontal' && e.key === 'left' ||
						this.options.orientation === 'vertical' && e.key === this.options.direction.yInverse 
					){
						//move focus to the previous link if possible, if not, move focus to the last link in the menu
						if(this.btn.getParent().getPrevious()){
							this.btn.getParent().getPrevious().getFirst().focus();
						}else{
							this.btn.getParent().getParent().getLast().getFirst().focus();
						}
					}else if(
						this.options.orientation === 'horizontal' && e.key === 'right' ||
						this.options.orientation === 'vertical' && e.key === this.options.direction.y 
					){
						//move focus to the next link if possible, if not, move focus to the first link in the menu
						if (this.btn.getParent().getNext()) {
							this.btn.getParent().getNext().getFirst().focus();
						}else{
							this.btn.getParent().getParent().getFirst().getFirst().focus();
						}
					}
				}else{
					if(e.key === 'tab'){e.stop();}
					//submenu parent buttons
					if (e.key === 'up') {
						if (this.btn.getParent('li').getPrevious('li')) {
							//move focus to the next link up
							this.btn.getParent('li').getPrevious('li').getFirst('a').focus();
						}else if(this.options.direction.y === 'down'){
							//move focus to the parent link
							this.parentSubMenu.btn.focus();
						}else if(this.options.direction.y === 'up'){
							//move focus to the bottom link in this submenu
							this.btn.getParent('li').getParent().getLast('li').getFirst('a').focus();
						}
					}else if(e.key === 'down'){
						if(this.btn.getParent('li').getNext('li')){
							//move focus to the next link down
							this.btn.getParent('li').getNext('li').getFirst('a').focus();
						}else if(this.options.direction.y === 'down'){
							//move focus to the top link in this submenu
							this.btn.getParent('li').getParent().getFirst('li').getFirst('a').focus();
						}else if(this.options.direction.y === 'up'){
							//move focus to the parent link
							this.parentSubMenu.btn.focus();
						}
					}else if(e.key === this.options.direction.xInverse){
						this.parentSubMenu.btn.focus();
					}else if(e.key === this.options.direction.x){
						if(this.options.direction.y === 'down'){
							this.childMenu.getFirst().getFirst('li').getFirst('a').focus();
						}else if(this.options.direction.y === 'up'){
						//	this.childMenu.getFirst().getLast('li').getFirst('a').focus();
						}
					}
				}
			}.bind(this)	
		});
		
		this.options.onSubMenuInit_complete(this);
		
    },
	
	matchWidth:function(){
		if (this.widthMatched || !this.options.matchWidthMode || this.subMenuType === 'subsequent'){return;}
		this.options.onMatchWidth_begin(this);
		var parentWidth = this.btn.getCoordinates().width;
		$(this.childMenu).getElements('a').each(function(item,index){
			var borderWidth = parseFloat($(this.childMenu).getFirst().getStyle('border-left-width')) + parseFloat($(this.childMenu).getFirst().getStyle('border-right-width'));
			var paddingWidth = parseFloat(item.getStyle('padding-left')) +	 parseFloat(item.getStyle('padding-right'));
			var offset = borderWidth + paddingWidth ;
			if(parentWidth > item.getCoordinates().width){
				item.setStyle('width',parentWidth - offset);
				item.setStyle('margin-right',-borderWidth);
			}
		}.bind(this));
		this.width = this.childMenu.getFirst().getCoordinates().width;
		this.widthMatched = true;
		this.options.onMatchWidth_complete(this);
	},
	
	hideSubMenu: function() {	
		if(this.childMenu.retrieve('status') === 'closed'){return;}	
		this.options.onHideSubMenu_begin(this);
		if (this.subMenuType == 'initial') {
			if(this.options.mmbClassName && this.options.mmbFocusedClassName){
				$(this.btn).retrieve('btnMorph', new Fx.Morph($(this.btn), { 'duration':(this.options.duration), transition:this.options.physics, link:'cancel' })).start(this.options.mmbClassName )
				.chain(function(){
					$(this.btn).removeClass('mainMenuParentBtnFocused');
					$(this.btn).addClass('mainMenuParentBtn');
				}.bind(this));
			}else{
				$(this.btn).removeClass('mainMenuParentBtnFocused');
				$(this.btn).addClass('mainMenuParentBtn');
			}
		}else{
			$(this.btn).removeClass('subMenuParentBtnFocused');
			$(this.btn).addClass('subMenuParentBtn');
		}
		
		this.childMenu.setStyle('z-index',1);
		
		if(this.options.effect && this.options.effect.toLowerCase() === 'slide'){
			if (this.subMenuType == 'initial' && this.options.orientation === 'horizontal' && this.options.direction.y === 'down') {
				this.myEffect.start({ 'margin-top': -this.height }).chain(function(){	this.childMenu.style.display = "none";	}.bind(this));
			}else if (this.subMenuType == 'initial' && this.options.orientation === 'horizontal' && this.options.direction.y === 'up') {
				this.myEffect.start({ 'margin-top': this.height }).chain(function(){	this.childMenu.style.display = "none";	}.bind(this));
			}else if(this.options.direction.x === 'right'){
				this.myEffect.start({ 'margin-left': -this.width }).chain(function(){	this.childMenu.style.display = "none";	}.bind(this));
			}else if(this.options.direction.x === 'left'){
				this.myEffect.start({ 'margin-left': this.width }).chain(function(){	this.childMenu.style.display = "none";	}.bind(this));
			}
		}else if(this.options.effect == 'fade'){
			this.myEffect.start({ 'opacity': 0 }).chain(function(){	this.childMenu.style.display = "none";	}.bind(this));
		}else if(this.options.effect == 'slide & fade'){
			
			if (this.subMenuType == 'initial' && this.options.orientation === 'horizontal' && this.options.direction.y === 'down') {
				this.myEffect.start({ 'margin-top': -this.height,opacity:0 }).chain(function(){	this.childMenu.style.display = "none";	}.bind(this));
			}else if (this.subMenuType == 'initial' && this.options.orientation === 'horizontal' && this.options.direction.y === 'up') {
				this.myEffect.start({ 'margin-top': this.height,opacity:0 }).chain(function(){	this.childMenu.style.display = "none";	}.bind(this));
			}else if(this.options.direction.x === 'right'){
				this.myEffect.start({ 'margin-left': -this.width,opacity:0 }).chain(function(){	this.childMenu.style.display = "none";	}.bind(this));
			}else if(this.options.direction.x === 'left'){
				this.myEffect.start({ 'margin-left': this.width, opacity:0 }).chain(function(){	this.childMenu.style.display = "none";	}.bind(this));
			}
		}else{
			this.childMenu.style.display = "none";
		}
		this.childMenu.store('status','closed');
		this.options.onHideSubMenu_complete(this);
	},
	
	hideOtherSubMenus: function() {		
		this.options.onHideOtherSubMenus_begin(this);
		//set up otherSubMenus element collection
		if(!this.btn.retrieve('otherSubMenus')){
			this.btn.store('otherSubMenus', $$(this.root.allSubMenus.filter(function(item){ return !this.btn.retrieve('parentSubMenus').contains(item) && item != this.childMenu; }.bind(this)) ));
		}		
		this.parentSubMenus.fireEvent('show');
		this.btn.retrieve('otherSubMenus').fireEvent('hide');
		this.options.onHideOtherSubMenus_complete(this);
	},
	
	hideAllSubMenus: function(){
		this.options.onHideAllSubMenus_begin(this);
		$clear(this.root.hideAllMenusTimeout);
		this.root.hideAllMenusTimeout = (function(){
			$clear(this.hideAllMenusTimeout);			
			$$(this.root.allSubMenus).fireEvent('hide');			
		}).bind(this).delay(this.options.hideDelay);
		this.options.onHideAllSubMenus_complete(this);		
	},

	cancellHideAllSubMenus: function(){ 
		$clear(this.root.hideAllMenusTimeout);	
	},
	
	showSubMenu: function(now){		
		if(this.childMenu.retrieve('status') === 'open'){return;}
		this.options.onShowSubMenu_begin(this);
		if (this.subMenuType == 'initial') {
			$(this.btn).removeClass('mainMenuParentBtn');
			$(this.btn).addClass('mainMenuParentBtnFocused');	
		}else{
			$(this.btn).removeClass('subMenuParentBtn');
			$(this.btn).addClass('subMenuParentBtnFocused');
		}
		this.root.subMenuZindex++;
		this.childMenu.setStyles({'display':'block','visibility':'hidden','z-index':this.root.subMenuZindex});
		
		if(!this.width || !this.height ){
			this.width = this.childMenu.getFirst().getCoordinates().width;
			this.height = this.childMenu.getFirst().getCoordinates().height;
			this.childMenu.setStyle('height',this.height,'border');
			if(this.options.effect === 'slide' || this.options.effect === 'slide & fade'){
				if (this.subMenuType == 'initial' && this.options.orientation === 'horizontal' ) {
					this.childMenu.getFirst().setStyle('margin-top','0' );
					if(this.options.direction.y === 'down'){
						this.myEffect.set({ 'margin-top': - this.height });
					}else if(this.options.direction.y === 'up'){
						this.myEffect.set({ 'margin-top': this.height });
					}
				}else {
					if(this.options.direction.x === 'left'){
						this.myEffect.set({ 'margin-left': this.width });
					}else{
						this.myEffect.set({ 'margin-left': -this.width });
					}
				}
			}
		}
		this.matchWidth();
		this.positionSubMenu();
		
		if(this.options.effect === 'slide' ){
			this.childMenu.setStyles({'display':'block','visibility':'visible'});
			if (this.subMenuType === 'initial' && this.options.orientation === 'horizontal') {
				if(now){
					this.myEffect.set({ 'margin-top': 0 }).chain(function(){	this.showSubMenuComplete();	}.bind(this));
				}else{
					this.myEffect.start({ 'margin-top': 0 }).chain(function(){	this.showSubMenuComplete();	}.bind(this));
				}
			}else{
				if (now) {
					this.myEffect.set({ 'margin-left': 0 }).chain(function(){	this.showSubMenuComplete();	}.bind(this));
				}else{
					this.myEffect.start({ 'margin-left': 0 }).chain(function(){	this.showSubMenuComplete();	}.bind(this));
				}
			}
		}else if(this.options.effect === 'fade' ){
			if (now) {
				this.myEffect.set({'opacity': this.options.opacity}).chain(function(){	this.showSubMenuComplete();	}.bind(this));
			}else{
				this.myEffect.start({'opacity': this.options.opacity}).chain(function(){	this.showSubMenuComplete();	}.bind(this));
			}
		}else if(this.options.effect == 'slide & fade'){
			this.childMenu.setStyles({'display':'block','visibility':'visible'});
			this.childMenu.getFirst().setStyles({'left':0});
			if (this.subMenuType === 'initial' && this.options.orientation === 'horizontal') {
				if (now) {
					this.myEffect.set({ 'margin-top': 0, 'opacity': this.options.opacity }).chain(function(){	this.showSubMenuComplete();	}.bind(this));
				}else{
					this.myEffect.start({ 'margin-top': 0, 'opacity': this.options.opacity }).chain(function(){	this.showSubMenuComplete();	}.bind(this));
				}
			}else{
				if (now) {
					if (this.options.direction.x === 'right') {
						this.myEffect.set({ 'margin-left': 0, 'opacity': this.options.opacity }).chain(function(){	this.showSubMenuComplete();	}.bind(this));
					}else if (this.options.direction.x === 'left') {
						this.myEffect.set({ 'margin-left': 0, 'opacity': this.options.opacity }).chain(function(){	this.showSubMenuComplete();	}.bind(this));
					}	
				}else{
					if (this.options.direction.x === 'right') {						
						this.myEffect.set({ 'margin-left': -this.width, 'opacity': this.options.opacity });						
						this.myEffect.start({ 'margin-left': 0, 'opacity': this.options.opacity }).chain(function(){	this.showSubMenuComplete();	}.bind(this));
					}else if (this.options.direction.x === 'left') {
						this.myEffect.start({ 'margin-left': 0, 'opacity': this.options.opacity }).chain(function(){	this.showSubMenuComplete();	}.bind(this));
					}
				}
			}
		}else{
			this.childMenu.setStyles({'display':'block','visibility':'visible'}).chain(function(){	this.showSubMenuComplete(this);	}.bind(this));
		}
		this.childMenu.store('status','open');
		
	},
	
	showSubMenuComplete:function(){		
		this.options.onShowSubMenu_complete(this);
	},
	
	positionSubMenu: function(){
		this.options.onPositionSubMenu_begin(this);
		this.childMenu.setStyle('width',this.width) ;
		this.childMenu.getFirst().setStyle('width',this.width) ;
				
		//if any parent has bounced off a viewport edge, inherit that new direction
		if (this.subMenuType === 'subsequent') {
			if(this.parentSubMenu && this.options.direction.x != this.parentSubMenu.options.direction.x){
				if(this.parentSubMenu.options.direction.x === 'left' && this.options.effect && this.options.effect.contains('slide')){
					this.myEffect.set({ 'margin-left': this.width });	
				}
			}
			this.options.direction.x = this.parentSubMenu.options.direction.x;
			this.options.direction.xInverse = this.parentSubMenu.options.direction.xInverse;
			this.options.direction.y = this.parentSubMenu.options.direction.y;
			this.options.direction.yInverse = this.parentSubMenu.options.direction.yInverse;
		}

		var top;
		var overlap
		if(this.subMenuType == 'initial'){
			if(	this.options.direction.y === 'up'){				
				if(this.options.orientation === 'vertical'){
					top = this.btn.getCoordinates().bottom - this.height + this.options.tweakInitial.y ;
				}else{			
					top = this.btn.getCoordinates().top - this.height + this.options.tweakInitial.y ;
				}
				this.childMenu.style.top = top+ 'px';
			}else if(this.options.orientation == 'horizontal'){
				this.childMenu.style.top = this.btn.getCoordinates().bottom + this.options.tweakInitial.y + 'px';
			}else if(this.options.orientation == 'vertical'){
				top = this.btn.getPosition().y + this.options.tweakInitial.y ;				
				if((top + this.childMenu.getSize().y) >= $(document.body).getScrollSize().y){
					overlap = (top + this.childMenu.getSize().y) - $(document.body).getScrollSize().y  ;
					top = top - overlap - 20;
				}	
				this.childMenu.style.top = top+ 'px';
			}
			if(	this.options.orientation == 'horizontal'){
				this.childMenu.style.left = this.btn.getPosition().x + this.options.tweakInitial.x + 'px';
			}else if(this.options.direction.x == 'left'){
				this.childMenu.style.left = this.btn.getPosition().x - this.childMenu.getCoordinates().width + this.options.tweakInitial.x + 'px';
			}else if(this.options.direction.x == 'right'){
				this.childMenu.style.left = this.btn.getCoordinates().right + this.options.tweakInitial.x + 'px';
			}
		}else if(this.subMenuType == 'subsequent'){
			
			if(this.options.direction.y === 'down'){
				if( (this.btn.getCoordinates().top + this.options.tweakSubsequent.y+ this.childMenu.getSize().y) >= $(document.body).getScrollSize().y ){
					overlap =  (this.btn.getCoordinates().top + this.options.tweakSubsequent.y+ this.childMenu.getSize().y) -$(document.body).getScrollSize().y  ;
					this.childMenu.style.top = (this.btn.getCoordinates().top + this.options.tweakSubsequent.y) - overlap - 20+ 'px';
				}else{
					this.childMenu.style.top = this.btn.getCoordinates().top + this.options.tweakSubsequent.y + 'px';
				}
			}else if(this.options.direction.y === 'up'){
				if((this.btn.getCoordinates().bottom - this.height + this.options.tweakSubsequent.y)< 1){
					this.options.direction.y = 'down';
					this.options.direction.yInverse = 'up';
					this.childMenu.style.top = this.btn.getCoordinates().top + this.options.tweakSubsequent.y + 'px';
				}else{
					this.childMenu.style.top = this.btn.getCoordinates().bottom - this.height + this.options.tweakSubsequent.y + 'px';
				}
			}
			if(this.options.direction.x == 'left'){
				this.childMenu.style.left = this.btn.getCoordinates().left - this.childMenu.getCoordinates().width + this.options.tweakSubsequent.x + 'px';
				
				if( this.childMenu.getPosition().x < 0){					
					this.options.direction.x = 'right';
					this.options.direction.xInverse = 'left';
					this.childMenu.style.left = this.btn.getPosition().x + this.btn.getCoordinates().width + this.options.tweakSubsequent.x + 'px';
					
					if(this.options.effect === 'slide' || this.options.effect === 'slide & fade'){
						this.myEffect.set({ 'margin-left': -this.width, 'opacity': this.options.opacity });						
					}
				}
			}else if(this.options.direction.x == 'right'){
				this.childMenu.style.left = this.btn.getCoordinates().right + this.options.tweakSubsequent.x + 'px';
				var smRight = this.childMenu.getCoordinates().right;
				var viewportRightEdge = document.getCoordinates().width + window.getScroll().x;				
				if( smRight > viewportRightEdge ){
					this.options.direction.x = 'left';
					this.options.direction.xInverse = 'right';
					this.childMenu.style.left = this.btn.getCoordinates().left - this.childMenu.getCoordinates().width + this.options.tweakSubsequent.x + 'px'
					if (this.options.effect === 'slide' || this.options.effect === 'slide & fade') {
						this.myEffect.set({	'margin-left': this.width,	'opacity': this.options.opacity	});
					}
				}
			}
		}
		this.options.onPositionSubMenu_complete(this);
	}	
});
