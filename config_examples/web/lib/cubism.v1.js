



<!DOCTYPE html>
<html lang="en" class="">
  <head prefix="og: http://ogp.me/ns# fb: http://ogp.me/ns/fb# object: http://ogp.me/ns/object# article: http://ogp.me/ns/article# profile: http://ogp.me/ns/profile#">
    <meta charset='utf-8'>
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta http-equiv="Content-Language" content="en">
    
    
    <title>cubism/cubism.v1.js at master · square/cubism</title>
    <link rel="search" type="application/opensearchdescription+xml" href="/opensearch.xml" title="GitHub">
    <link rel="fluid-icon" href="https://github.com/fluidicon.png" title="GitHub">
    <link rel="apple-touch-icon" sizes="57x57" href="/apple-touch-icon-114.png">
    <link rel="apple-touch-icon" sizes="114x114" href="/apple-touch-icon-114.png">
    <link rel="apple-touch-icon" sizes="72x72" href="/apple-touch-icon-144.png">
    <link rel="apple-touch-icon" sizes="144x144" href="/apple-touch-icon-144.png">
    <meta property="fb:app_id" content="1401488693436528">

      <meta content="@github" name="twitter:site" /><meta content="summary" name="twitter:card" /><meta content="square/cubism" name="twitter:title" /><meta content="cubism - Cubism.js: A JavaScript library for time series visualization." name="twitter:description" /><meta content="https://avatars0.githubusercontent.com/u/82592?v=2&amp;s=400" name="twitter:image:src" />
<meta content="GitHub" property="og:site_name" /><meta content="object" property="og:type" /><meta content="https://avatars0.githubusercontent.com/u/82592?v=2&amp;s=400" property="og:image" /><meta content="square/cubism" property="og:title" /><meta content="https://github.com/square/cubism" property="og:url" /><meta content="cubism - Cubism.js: A JavaScript library for time series visualization." property="og:description" />

      <meta name="browser-stats-url" content="/_stats">
    <link rel="assets" href="https://assets-cdn.github.com/">
    <link rel="conduit-xhr" href="https://ghconduit.com:25035">
    <link rel="xhr-socket" href="/_sockets">
    <meta name="pjax-timeout" content="1000">

    <meta name="msapplication-TileImage" content="/windows-tile.png">
    <meta name="msapplication-TileColor" content="#ffffff">
    <meta name="selected-link" value="repo_source" data-pjax-transient>
      <meta name="google-analytics" content="UA-3769691-2">

    <meta content="collector.githubapp.com" name="octolytics-host" /><meta content="collector-cdn.github.com" name="octolytics-script-host" /><meta content="github" name="octolytics-app-id" /><meta content="57F9A6D4:2A3C:8D5473:543E99B2" name="octolytics-dimension-request_id" /><meta content="1358883" name="octolytics-actor-id" /><meta content="grodansparadis" name="octolytics-actor-login" /><meta content="5fe117fae795a1b0ec64a69e6315e685e7549e21ea66bdd8570a9f9f9459ebbd" name="octolytics-actor-hash" />
    <meta content="Rails, view, blob#show" name="analytics-event" />

    
    
    <link rel="icon" type="image/x-icon" href="https://assets-cdn.github.com/favicon.ico">


    <meta content="authenticity_token" name="csrf-param" />
<meta content="B0f8cdLR0CEyDOUTux6HYIxfNj8L29VadOfrdAT2zsw67QP37IuJaxP1DZvE5otss+MiiFA+XrAucZCCJ+YU3Q==" name="csrf-token" />

    <link href="https://assets-cdn.github.com/assets/github-0771b929a54c98ad48c1075a82c42e8cd8afb8516f8b4f3da76a461695daaf80.css" media="all" rel="stylesheet" type="text/css" />
    <link href="https://assets-cdn.github.com/assets/github2-c7c5c123bf8810b1bccded004dcae7b61fd9e418d4e372478b2b30c900f1ab24.css" media="all" rel="stylesheet" type="text/css" />
    
    


    <meta http-equiv="x-pjax-version" content="fee2f1d4f3804b285e956a15df318b0e">

      
  <meta name="description" content="cubism - Cubism.js: A JavaScript library for time series visualization.">
  <meta name="go-import" content="github.com/square/cubism git https://github.com/square/cubism.git">

  <meta content="82592" name="octolytics-dimension-user_id" /><meta content="square" name="octolytics-dimension-user_login" /><meta content="3859306" name="octolytics-dimension-repository_id" /><meta content="square/cubism" name="octolytics-dimension-repository_nwo" /><meta content="true" name="octolytics-dimension-repository_public" /><meta content="false" name="octolytics-dimension-repository_is_fork" /><meta content="3859306" name="octolytics-dimension-repository_network_root_id" /><meta content="square/cubism" name="octolytics-dimension-repository_network_root_nwo" />
  <link href="https://github.com/square/cubism/commits/master.atom" rel="alternate" title="Recent Commits to cubism:master" type="application/atom+xml">

  </head>


  <body class="logged_in  env-production windows vis-public page-blob">
    <a href="#start-of-content" tabindex="1" class="accessibility-aid js-skip-to-content">Skip to content</a>
    <div class="wrapper">
      
      
      
      


      <div class="header header-logged-in true" role="banner">
  <div class="container clearfix">

    <a class="header-logo-invertocat" href="https://github.com/" data-hotkey="g d" aria-label="Homepage" ga-data-click="Header, go to dashboard, icon:logo">
  <span class="mega-octicon octicon-mark-github"></span>
</a>


      <div class="site-search repo-scope js-site-search" role="search">
          <form accept-charset="UTF-8" action="/square/cubism/search" class="js-site-search-form" data-global-search-url="/search" data-repo-search-url="/square/cubism/search" method="get"><div style="margin:0;padding:0;display:inline"><input name="utf8" type="hidden" value="&#x2713;" /></div>
  <input type="text"
    class="js-site-search-field is-clearable"
    data-hotkey="s"
    name="q"
    placeholder="Search"
    data-global-scope-placeholder="Search GitHub"
    data-repo-scope-placeholder="Search"
    tabindex="1"
    autocapitalize="off">
  <div class="scope-badge">This repository</div>
</form>
      </div>
      <ul class="header-nav left" role="navigation">
        <li class="header-nav-item explore">
          <a class="header-nav-link" href="/explore" data-ga-click="Header, go to explore, text:explore">Explore</a>
        </li>
          <li class="header-nav-item">
            <a class="header-nav-link" href="https://gist.github.com" data-ga-click="Header, go to gist, text:gist">Gist</a>
          </li>
          <li class="header-nav-item">
            <a class="header-nav-link" href="/blog" data-ga-click="Header, go to blog, text:blog">Blog</a>
          </li>
        <li class="header-nav-item">
          <a class="header-nav-link" href="https://help.github.com" data-ga-click="Header, go to help, text:help">Help</a>
        </li>
      </ul>

    
<ul class="header-nav user-nav right" id="user-links">
  <li class="header-nav-item dropdown js-menu-container">
    <a class="header-nav-link name" href="/grodansparadis" data-ga-click="Header, go to profile, text:username">
      <img alt="Ake Hedman" class="avatar" data-user="1358883" height="20" src="https://avatars2.githubusercontent.com/u/1358883?v=2&amp;s=40" width="20" />
      <span class="css-truncate">
        <span class="css-truncate-target">grodansparadis</span>
      </span>
    </a>
  </li>

  <li class="header-nav-item dropdown js-menu-container">
    <a class="header-nav-link js-menu-target tooltipped tooltipped-s" href="#" aria-label="Create new..." data-ga-click="Header, create new, icon:add">
      <span class="octicon octicon-plus"></span>
      <span class="dropdown-caret"></span>
    </a>

    <div class="dropdown-menu-content js-menu-content">
      
<ul class="dropdown-menu">
  <li>
    <a href="/new"><span class="octicon octicon-repo"></span> New repository</a>
  </li>
  <li>
    <a href="/organizations/new"><span class="octicon octicon-organization"></span> New organization</a>
  </li>


    <li class="dropdown-divider"></li>
    <li class="dropdown-header">
      <span title="square/cubism">This repository</span>
    </li>
      <li>
        <a href="/square/cubism/issues/new"><span class="octicon octicon-issue-opened"></span> New issue</a>
      </li>
</ul>

    </div>
  </li>

  <li class="header-nav-item">
        <a href="/notifications" aria-label="You have unread notifications" class="header-nav-link notification-indicator tooltipped tooltipped-s" data-ga-click="Header, go to notifications, icon:unread" data-hotkey="g n">
        <span class="mail-status unread"></span>
        <span class="octicon octicon-inbox"></span>
</a>
  </li>

  <li class="header-nav-item">
    <a class="header-nav-link tooltipped tooltipped-s" href="/settings/profile" id="account_settings" aria-label="Settings" data-ga-click="Header, go to settings, icon:settings">
      <span class="octicon octicon-gear"></span>
    </a>
  </li>

  <li class="header-nav-item">
    <form accept-charset="UTF-8" action="/logout" class="logout-form" method="post"><div style="margin:0;padding:0;display:inline"><input name="utf8" type="hidden" value="&#x2713;" /><input name="authenticity_token" type="hidden" value="MvWT4g/JmvFgCPsvs1Sg5hDblDCgIuS14EcmoWV7LxgTpIuYpoxkt1E+olC2UXke6JAlqA/9K+L84eLMACMvkg==" /></div>
      <button class="header-nav-link sign-out-button tooltipped tooltipped-s" aria-label="Sign out" data-ga-click="Header, sign out, icon:logout">
        <span class="octicon octicon-sign-out"></span>
      </button>
</form>  </li>

</ul>


    
  </div>
</div>

      

        


      <div id="start-of-content" class="accessibility-aid"></div>
          <div class="site" itemscope itemtype="http://schema.org/WebPage">
    <div id="js-flash-container">
      
    </div>
    <div class="pagehead repohead instapaper_ignore readability-menu">
      <div class="container">
        
<ul class="pagehead-actions">

    <li class="subscription">
      <form accept-charset="UTF-8" action="/notifications/subscribe" class="js-social-container" data-autosubmit="true" data-remote="true" method="post"><div style="margin:0;padding:0;display:inline"><input name="utf8" type="hidden" value="&#x2713;" /><input name="authenticity_token" type="hidden" value="Gm7NEqvZrjiFJvCLL89gA3HP8AdyicnytkISlmZpQeRJxj/JV2D8+om2+Ca/uTKuEvox5cbx7Q0KYPwycXMFZA==" /></div>  <input id="repository_id" name="repository_id" type="hidden" value="3859306" />

    <div class="select-menu js-menu-container js-select-menu">
      <a class="social-count js-social-count" href="/square/cubism/watchers">
        147
      </a>
      <a href="/square/cubism/subscription"
        class="minibutton select-menu-button with-count js-menu-target" role="button" tabindex="0" aria-haspopup="true">
        <span class="js-select-button">
          <span class="octicon octicon-eye"></span>
          Watch
        </span>
      </a>

      <div class="select-menu-modal-holder">
        <div class="select-menu-modal subscription-menu-modal js-menu-content" aria-hidden="true">
          <div class="select-menu-header">
            <span class="select-menu-title">Notifications</span>
            <span class="octicon octicon-x js-menu-close" role="button" aria-label="Close"></span>
          </div> <!-- /.select-menu-header -->

          <div class="select-menu-list js-navigation-container" role="menu">

            <div class="select-menu-item js-navigation-item selected" role="menuitem" tabindex="0">
              <span class="select-menu-item-icon octicon octicon-check"></span>
              <div class="select-menu-item-text">
                <input checked="checked" id="do_included" name="do" type="radio" value="included" />
                <h4>Not watching</h4>
                <span class="description">Be notified when participating or @mentioned.</span>
                <span class="js-select-button-text hidden-select-button-text">
                  <span class="octicon octicon-eye"></span>
                  Watch
                </span>
              </div>
            </div> <!-- /.select-menu-item -->

            <div class="select-menu-item js-navigation-item " role="menuitem" tabindex="0">
              <span class="select-menu-item-icon octicon octicon octicon-check"></span>
              <div class="select-menu-item-text">
                <input id="do_subscribed" name="do" type="radio" value="subscribed" />
                <h4>Watching</h4>
                <span class="description">Be notified of all conversations.</span>
                <span class="js-select-button-text hidden-select-button-text">
                  <span class="octicon octicon-eye"></span>
                  Unwatch
                </span>
              </div>
            </div> <!-- /.select-menu-item -->

            <div class="select-menu-item js-navigation-item " role="menuitem" tabindex="0">
              <span class="select-menu-item-icon octicon octicon-check"></span>
              <div class="select-menu-item-text">
                <input id="do_ignore" name="do" type="radio" value="ignore" />
                <h4>Ignoring</h4>
                <span class="description">Never be notified.</span>
                <span class="js-select-button-text hidden-select-button-text">
                  <span class="octicon octicon-mute"></span>
                  Stop ignoring
                </span>
              </div>
            </div> <!-- /.select-menu-item -->

          </div> <!-- /.select-menu-list -->

        </div> <!-- /.select-menu-modal -->
      </div> <!-- /.select-menu-modal-holder -->
    </div> <!-- /.select-menu -->

</form>
    </li>

  <li>
    
  <div class="js-toggler-container js-social-container starring-container ">

    <form accept-charset="UTF-8" action="/square/cubism/unstar" class="js-toggler-form starred js-unstar-button" data-remote="true" method="post"><div style="margin:0;padding:0;display:inline"><input name="utf8" type="hidden" value="&#x2713;" /><input name="authenticity_token" type="hidden" value="NM5EtKbjpmuMt2yNWFI9P/r4f69h4+mOGZyxmGaeTLeD8kENRkhRTwfmc3wCxGdMSyYUfwsv0Ig4+GnbVONDzw==" /></div>
      <button
        class="minibutton with-count js-toggler-target star-button"
        aria-label="Unstar this repository" title="Unstar square/cubism">
        <span class="octicon octicon-star"></span>
        Unstar
      </button>
        <a class="social-count js-social-count" href="/square/cubism/stargazers">
          3,540
        </a>
</form>
    <form accept-charset="UTF-8" action="/square/cubism/star" class="js-toggler-form unstarred js-star-button" data-remote="true" method="post"><div style="margin:0;padding:0;display:inline"><input name="utf8" type="hidden" value="&#x2713;" /><input name="authenticity_token" type="hidden" value="HcTeUP4V63hHWRKnJENelPpKuI/2fKPyOS6O0tMBKaiVL69DnRyJ/z2RuowStTASge815EJ2sJDAczv360FTdg==" /></div>
      <button
        class="minibutton with-count js-toggler-target star-button"
        aria-label="Star this repository" title="Star square/cubism">
        <span class="octicon octicon-star"></span>
        Star
      </button>
        <a class="social-count js-social-count" href="/square/cubism/stargazers">
          3,540
        </a>
</form>  </div>

  </li>


        <li>
          <a href="/square/cubism/fork" class="minibutton with-count js-toggler-target fork-button tooltipped-n" title="Fork your own copy of square/cubism to your account" aria-label="Fork your own copy of square/cubism to your account" rel="nofollow" data-method="post">
            <span class="octicon octicon-repo-forked"></span>
            Fork
          </a>
          <a href="/square/cubism/network" class="social-count">358</a>
        </li>

</ul>

        <h1 itemscope itemtype="http://data-vocabulary.org/Breadcrumb" class="entry-title public">
          <span class="mega-octicon octicon-repo"></span>
          <span class="author"><a href="/square" class="url fn" itemprop="url" rel="author"><span itemprop="title">square</span></a></span><!--
       --><span class="path-divider">/</span><!--
       --><strong><a href="/square/cubism" class="js-current-repository js-repo-home-link">cubism</a></strong>

          <span class="page-context-loader">
            <img alt="" height="16" src="https://assets-cdn.github.com/images/spinners/octocat-spinner-32.gif" width="16" />
          </span>

        </h1>
      </div><!-- /.container -->
    </div><!-- /.repohead -->

    <div class="container">
      <div class="repository-with-sidebar repo-container new-discussion-timeline  ">
        <div class="repository-sidebar clearfix">
            
<div class="sunken-menu vertical-right repo-nav js-repo-nav js-repository-container-pjax js-octicon-loaders" role="navigation" data-issue-count-url="/square/cubism/issues/counts">
  <div class="sunken-menu-contents">
    <ul class="sunken-menu-group">
      <li class="tooltipped tooltipped-w" aria-label="Code">
        <a href="/square/cubism" aria-label="Code" class="selected js-selected-navigation-item sunken-menu-item" data-hotkey="g c" data-pjax="true" data-selected-links="repo_source repo_downloads repo_commits repo_releases repo_tags repo_branches /square/cubism">
          <span class="octicon octicon-code"></span> <span class="full-word">Code</span>
          <img alt="" class="mini-loader" height="16" src="https://assets-cdn.github.com/images/spinners/octocat-spinner-32.gif" width="16" />
</a>      </li>

        <li class="tooltipped tooltipped-w" aria-label="Issues">
          <a href="/square/cubism/issues" aria-label="Issues" class="js-selected-navigation-item sunken-menu-item js-disable-pjax" data-hotkey="g i" data-selected-links="repo_issues repo_labels repo_milestones /square/cubism/issues">
            <span class="octicon octicon-issue-opened"></span> <span class="full-word">Issues</span>
            <span class="js-issue-replace-counter"></span>
            <img alt="" class="mini-loader" height="16" src="https://assets-cdn.github.com/images/spinners/octocat-spinner-32.gif" width="16" />
</a>        </li>

      <li class="tooltipped tooltipped-w" aria-label="Pull Requests">
        <a href="/square/cubism/pulls" aria-label="Pull Requests" class="js-selected-navigation-item sunken-menu-item js-disable-pjax" data-hotkey="g p" data-selected-links="repo_pulls /square/cubism/pulls">
            <span class="octicon octicon-git-pull-request"></span> <span class="full-word">Pull Requests</span>
            <span class="js-pull-replace-counter"></span>
            <img alt="" class="mini-loader" height="16" src="https://assets-cdn.github.com/images/spinners/octocat-spinner-32.gif" width="16" />
</a>      </li>


        <li class="tooltipped tooltipped-w" aria-label="Wiki">
          <a href="/square/cubism/wiki" aria-label="Wiki" class="js-selected-navigation-item sunken-menu-item js-disable-pjax" data-hotkey="g w" data-selected-links="repo_wiki /square/cubism/wiki">
            <span class="octicon octicon-book"></span> <span class="full-word">Wiki</span>
            <img alt="" class="mini-loader" height="16" src="https://assets-cdn.github.com/images/spinners/octocat-spinner-32.gif" width="16" />
</a>        </li>
    </ul>
    <div class="sunken-menu-separator"></div>
    <ul class="sunken-menu-group">

      <li class="tooltipped tooltipped-w" aria-label="Pulse">
        <a href="/square/cubism/pulse/weekly" aria-label="Pulse" class="js-selected-navigation-item sunken-menu-item" data-pjax="true" data-selected-links="pulse /square/cubism/pulse/weekly">
          <span class="octicon octicon-pulse"></span> <span class="full-word">Pulse</span>
          <img alt="" class="mini-loader" height="16" src="https://assets-cdn.github.com/images/spinners/octocat-spinner-32.gif" width="16" />
</a>      </li>

      <li class="tooltipped tooltipped-w" aria-label="Graphs">
        <a href="/square/cubism/graphs" aria-label="Graphs" class="js-selected-navigation-item sunken-menu-item" data-pjax="true" data-selected-links="repo_graphs repo_contributors /square/cubism/graphs">
          <span class="octicon octicon-graph"></span> <span class="full-word">Graphs</span>
          <img alt="" class="mini-loader" height="16" src="https://assets-cdn.github.com/images/spinners/octocat-spinner-32.gif" width="16" />
</a>      </li>
    </ul>


  </div>
</div>

              <div class="only-with-full-nav">
                
  
<div class="clone-url open"
  data-protocol-type="http"
  data-url="/users/set_protocol?protocol_selector=http&amp;protocol_type=clone">
  <h3><span class="text-emphasized">HTTPS</span> clone URL</h3>
  <div class="input-group">
    <input type="text" class="input-mini input-monospace js-url-field"
           value="https://github.com/square/cubism.git" readonly="readonly">
    <span class="input-group-button">
      <button aria-label="Copy to clipboard" class="js-zeroclipboard minibutton zeroclipboard-button" data-clipboard-text="https://github.com/square/cubism.git" data-copied-hint="Copied!" type="button"><span class="octicon octicon-clippy"></span></button>
    </span>
  </div>
</div>

  
<div class="clone-url "
  data-protocol-type="ssh"
  data-url="/users/set_protocol?protocol_selector=ssh&amp;protocol_type=clone">
  <h3><span class="text-emphasized">SSH</span> clone URL</h3>
  <div class="input-group">
    <input type="text" class="input-mini input-monospace js-url-field"
           value="git@github.com:square/cubism.git" readonly="readonly">
    <span class="input-group-button">
      <button aria-label="Copy to clipboard" class="js-zeroclipboard minibutton zeroclipboard-button" data-clipboard-text="git@github.com:square/cubism.git" data-copied-hint="Copied!" type="button"><span class="octicon octicon-clippy"></span></button>
    </span>
  </div>
</div>

  
<div class="clone-url "
  data-protocol-type="subversion"
  data-url="/users/set_protocol?protocol_selector=subversion&amp;protocol_type=clone">
  <h3><span class="text-emphasized">Subversion</span> checkout URL</h3>
  <div class="input-group">
    <input type="text" class="input-mini input-monospace js-url-field"
           value="https://github.com/square/cubism" readonly="readonly">
    <span class="input-group-button">
      <button aria-label="Copy to clipboard" class="js-zeroclipboard minibutton zeroclipboard-button" data-clipboard-text="https://github.com/square/cubism" data-copied-hint="Copied!" type="button"><span class="octicon octicon-clippy"></span></button>
    </span>
  </div>
</div>


<p class="clone-options">You can clone with
      <a href="#" class="js-clone-selector" data-protocol="http">HTTPS</a>,
      <a href="#" class="js-clone-selector" data-protocol="ssh">SSH</a>,
      or <a href="#" class="js-clone-selector" data-protocol="subversion">Subversion</a>.
  <a href="https://help.github.com/articles/which-remote-url-should-i-use" class="help tooltipped tooltipped-n" aria-label="Get help on which URL is right for you.">
    <span class="octicon octicon-question"></span>
  </a>
</p>


  <a href="github-windows://openRepo/https://github.com/square/cubism" class="minibutton sidebar-button" title="Save square/cubism to your computer and use it in GitHub Desktop." aria-label="Save square/cubism to your computer and use it in GitHub Desktop.">
    <span class="octicon octicon-device-desktop"></span>
    Clone in Desktop
  </a>

                <a href="/square/cubism/archive/master.zip"
                   class="minibutton sidebar-button"
                   aria-label="Download the contents of square/cubism as a zip file"
                   title="Download the contents of square/cubism as a zip file"
                   rel="nofollow">
                  <span class="octicon octicon-cloud-download"></span>
                  Download ZIP
                </a>
              </div>
        </div><!-- /.repository-sidebar -->

        <div id="js-repo-pjax-container" class="repository-content context-loader-container" data-pjax-container>
          

<a href="/square/cubism/blob/bda607fcd377d4a849db5c06fb129b26b3430913/cubism.v1.js" class="hidden js-permalink-shortcut" data-hotkey="y">Permalink</a>

<!-- blob contrib key: blob_contributors:v21:3e9325207506eea4e944e712fb16d5f8 -->

<div class="file-navigation">
  
<div class="select-menu js-menu-container js-select-menu left">
  <span class="minibutton select-menu-button js-menu-target css-truncate" data-hotkey="w"
    data-master-branch="master"
    data-ref="master"
    title="master"
    role="button" aria-label="Switch branches or tags" tabindex="0" aria-haspopup="true">
    <span class="octicon octicon-git-branch"></span>
    <i>branch:</i>
    <span class="js-select-button css-truncate-target">master</span>
  </span>

  <div class="select-menu-modal-holder js-menu-content js-navigation-container" data-pjax aria-hidden="true">

    <div class="select-menu-modal">
      <div class="select-menu-header">
        <span class="select-menu-title">Switch branches/tags</span>
        <span class="octicon octicon-x js-menu-close" role="button" aria-label="Close"></span>
      </div> <!-- /.select-menu-header -->

      <div class="select-menu-filters">
        <div class="select-menu-text-filter">
          <input type="text" aria-label="Filter branches/tags" id="context-commitish-filter-field" class="js-filterable-field js-navigation-enable" placeholder="Filter branches/tags">
        </div>
        <div class="select-menu-tabs">
          <ul>
            <li class="select-menu-tab">
              <a href="#" data-tab-filter="branches" class="js-select-menu-tab">Branches</a>
            </li>
            <li class="select-menu-tab">
              <a href="#" data-tab-filter="tags" class="js-select-menu-tab">Tags</a>
            </li>
          </ul>
        </div><!-- /.select-menu-tabs -->
      </div><!-- /.select-menu-filters -->

      <div class="select-menu-list select-menu-tab-bucket js-select-menu-tab-bucket" data-tab-filter="branches">

        <div data-filterable-for="context-commitish-filter-field" data-filterable-type="substring">


            <div class="select-menu-item js-navigation-item ">
              <span class="select-menu-item-icon octicon octicon-check"></span>
              <a href="/square/cubism/blob/cyberdelia-librato/cubism.v1.js"
                 data-name="cyberdelia-librato"
                 data-skip-pjax="true"
                 rel="nofollow"
                 class="js-navigation-open select-menu-item-text css-truncate-target"
                 title="cyberdelia-librato">cyberdelia-librato</a>
            </div> <!-- /.select-menu-item -->
            <div class="select-menu-item js-navigation-item ">
              <span class="select-menu-item-icon octicon octicon-check"></span>
              <a href="/square/cubism/blob/fix-mousemove-half-pixel/cubism.v1.js"
                 data-name="fix-mousemove-half-pixel"
                 data-skip-pjax="true"
                 rel="nofollow"
                 class="js-navigation-open select-menu-item-text css-truncate-target"
                 title="fix-mousemove-half-pixel">fix-mousemove-half-pixel</a>
            </div> <!-- /.select-menu-item -->
            <div class="select-menu-item js-navigation-item ">
              <span class="select-menu-item-icon octicon octicon-check"></span>
              <a href="/square/cubism/blob/gh-pages/cubism.v1.js"
                 data-name="gh-pages"
                 data-skip-pjax="true"
                 rel="nofollow"
                 class="js-navigation-open select-menu-item-text css-truncate-target"
                 title="gh-pages">gh-pages</a>
            </div> <!-- /.select-menu-item -->
            <div class="select-menu-item js-navigation-item selected">
              <span class="select-menu-item-icon octicon octicon-check"></span>
              <a href="/square/cubism/blob/master/cubism.v1.js"
                 data-name="master"
                 data-skip-pjax="true"
                 rel="nofollow"
                 class="js-navigation-open select-menu-item-text css-truncate-target"
                 title="master">master</a>
            </div> <!-- /.select-menu-item -->
            <div class="select-menu-item js-navigation-item ">
              <span class="select-menu-item-icon octicon octicon-check"></span>
              <a href="/square/cubism/blob/options/cubism.v1.js"
                 data-name="options"
                 data-skip-pjax="true"
                 rel="nofollow"
                 class="js-navigation-open select-menu-item-text css-truncate-target"
                 title="options">options</a>
            </div> <!-- /.select-menu-item -->
        </div>

          <div class="select-menu-no-results">Nothing to show</div>
      </div> <!-- /.select-menu-list -->

      <div class="select-menu-list select-menu-tab-bucket js-select-menu-tab-bucket" data-tab-filter="tags">
        <div data-filterable-for="context-commitish-filter-field" data-filterable-type="substring">


            <div class="select-menu-item js-navigation-item ">
              <span class="select-menu-item-icon octicon octicon-check"></span>
              <a href="/square/cubism/tree/v1.6.0/cubism.v1.js"
                 data-name="v1.6.0"
                 data-skip-pjax="true"
                 rel="nofollow"
                 class="js-navigation-open select-menu-item-text css-truncate-target"
                 title="v1.6.0">v1.6.0</a>
            </div> <!-- /.select-menu-item -->
            <div class="select-menu-item js-navigation-item ">
              <span class="select-menu-item-icon octicon octicon-check"></span>
              <a href="/square/cubism/tree/v1.2.2/cubism.v1.js"
                 data-name="v1.2.2"
                 data-skip-pjax="true"
                 rel="nofollow"
                 class="js-navigation-open select-menu-item-text css-truncate-target"
                 title="v1.2.2">v1.2.2</a>
            </div> <!-- /.select-menu-item -->
            <div class="select-menu-item js-navigation-item ">
              <span class="select-menu-item-icon octicon octicon-check"></span>
              <a href="/square/cubism/tree/v1.2.1/cubism.v1.js"
                 data-name="v1.2.1"
                 data-skip-pjax="true"
                 rel="nofollow"
                 class="js-navigation-open select-menu-item-text css-truncate-target"
                 title="v1.2.1">v1.2.1</a>
            </div> <!-- /.select-menu-item -->
            <div class="select-menu-item js-navigation-item ">
              <span class="select-menu-item-icon octicon octicon-check"></span>
              <a href="/square/cubism/tree/v1.2.0/cubism.v1.js"
                 data-name="v1.2.0"
                 data-skip-pjax="true"
                 rel="nofollow"
                 class="js-navigation-open select-menu-item-text css-truncate-target"
                 title="v1.2.0">v1.2.0</a>
            </div> <!-- /.select-menu-item -->
            <div class="select-menu-item js-navigation-item ">
              <span class="select-menu-item-icon octicon octicon-check"></span>
              <a href="/square/cubism/tree/v1.1.0/cubism.v1.js"
                 data-name="v1.1.0"
                 data-skip-pjax="true"
                 rel="nofollow"
                 class="js-navigation-open select-menu-item-text css-truncate-target"
                 title="v1.1.0">v1.1.0</a>
            </div> <!-- /.select-menu-item -->
            <div class="select-menu-item js-navigation-item ">
              <span class="select-menu-item-icon octicon octicon-check"></span>
              <a href="/square/cubism/tree/v1.0.1/cubism.v1.js"
                 data-name="v1.0.1"
                 data-skip-pjax="true"
                 rel="nofollow"
                 class="js-navigation-open select-menu-item-text css-truncate-target"
                 title="v1.0.1">v1.0.1</a>
            </div> <!-- /.select-menu-item -->
            <div class="select-menu-item js-navigation-item ">
              <span class="select-menu-item-icon octicon octicon-check"></span>
              <a href="/square/cubism/tree/v1.0.0/cubism.v1.js"
                 data-name="v1.0.0"
                 data-skip-pjax="true"
                 rel="nofollow"
                 class="js-navigation-open select-menu-item-text css-truncate-target"
                 title="v1.0.0">v1.0.0</a>
            </div> <!-- /.select-menu-item -->
            <div class="select-menu-item js-navigation-item ">
              <span class="select-menu-item-icon octicon octicon-check"></span>
              <a href="/square/cubism/tree/v0.1.0/cubism.v1.js"
                 data-name="v0.1.0"
                 data-skip-pjax="true"
                 rel="nofollow"
                 class="js-navigation-open select-menu-item-text css-truncate-target"
                 title="v0.1.0">v0.1.0</a>
            </div> <!-- /.select-menu-item -->
            <div class="select-menu-item js-navigation-item ">
              <span class="select-menu-item-icon octicon octicon-check"></span>
              <a href="/square/cubism/tree/semver/cubism.v1.js"
                 data-name="semver"
                 data-skip-pjax="true"
                 rel="nofollow"
                 class="js-navigation-open select-menu-item-text css-truncate-target"
                 title="semver">semver</a>
            </div> <!-- /.select-menu-item -->
        </div>

        <div class="select-menu-no-results">Nothing to show</div>
      </div> <!-- /.select-menu-list -->

    </div> <!-- /.select-menu-modal -->
  </div> <!-- /.select-menu-modal-holder -->
</div> <!-- /.select-menu -->

  <div class="button-group right">
    <a href="/square/cubism/find/master"
          class="js-show-file-finder minibutton empty-icon tooltipped tooltipped-s"
          data-pjax
          data-hotkey="t"
          aria-label="Quickly jump between files">
      <span class="octicon octicon-list-unordered"></span>
    </a>
    <button class="js-zeroclipboard minibutton zeroclipboard-button"
          data-clipboard-text="cubism.v1.js"
          aria-label="Copy to clipboard"
          data-copied-hint="Copied!">
      <span class="octicon octicon-clippy"></span>
    </button>
  </div>

  <div class="breadcrumb">
    <span class='repo-root js-repo-root'><span itemscope="" itemtype="http://data-vocabulary.org/Breadcrumb"><a href="/square/cubism" class="" data-branch="master" data-direction="back" data-pjax="true" itemscope="url"><span itemprop="title">cubism</span></a></span></span><span class="separator"> / </span><strong class="final-path">cubism.v1.js</strong>
  </div>
</div>


  <div class="commit file-history-tease">
    <div class="file-history-tease-header">
        <img alt="Tom Carden" class="avatar" data-user="39635" height="24" src="https://avatars3.githubusercontent.com/u/39635?v=2&amp;s=48" width="24" />
        <span class="author"><a href="/RandomEtc" rel="contributor">RandomEtc</a></span>
        <time datetime="2014-04-19T03:55:23Z" is="relative-time">Apr 18, 2014</time>
        <div class="commit-title">
            <a href="/square/cubism/commit/bda607fcd377d4a849db5c06fb129b26b3430913" class="message" data-pjax="true" title="rebuild cubism with new librato adapter; v1.6.0">rebuild cubism with new librato adapter; v1.6.0</a>
        </div>
    </div>

    <div class="participation">
      <p class="quickstat">
        <a href="#blob_contributors_box" rel="facebox">
          <strong>3</strong>
           contributors
        </a>
      </p>
          <a class="avatar-link tooltipped tooltipped-s" aria-label="mbostock" href="/square/cubism/commits/master/cubism.v1.js?author=mbostock"><img alt="Mike Bostock" class="avatar" data-user="230541" height="20" src="https://avatars3.githubusercontent.com/u/230541?v=2&amp;s=40" width="20" /></a>
    <a class="avatar-link tooltipped tooltipped-s" aria-label="RandomEtc" href="/square/cubism/commits/master/cubism.v1.js?author=RandomEtc"><img alt="Tom Carden" class="avatar" data-user="39635" height="20" src="https://avatars1.githubusercontent.com/u/39635?v=2&amp;s=40" width="20" /></a>
    <a class="avatar-link tooltipped tooltipped-s" aria-label="phobos182" href="/square/cubism/commits/master/cubism.v1.js?author=phobos182"><img alt="Jeremy Carroll" class="avatar" data-user="704234" height="20" src="https://avatars1.githubusercontent.com/u/704234?v=2&amp;s=40" width="20" /></a>


    </div>
    <div id="blob_contributors_box" style="display:none">
      <h2 class="facebox-header">Users who have contributed to this file</h2>
      <ul class="facebox-user-list">
          <li class="facebox-user-list-item">
            <img alt="Mike Bostock" data-user="230541" height="24" src="https://avatars1.githubusercontent.com/u/230541?v=2&amp;s=48" width="24" />
            <a href="/mbostock">mbostock</a>
          </li>
          <li class="facebox-user-list-item">
            <img alt="Tom Carden" data-user="39635" height="24" src="https://avatars3.githubusercontent.com/u/39635?v=2&amp;s=48" width="24" />
            <a href="/RandomEtc">RandomEtc</a>
          </li>
          <li class="facebox-user-list-item">
            <img alt="Jeremy Carroll" data-user="704234" height="24" src="https://avatars3.githubusercontent.com/u/704234?v=2&amp;s=48" width="24" />
            <a href="/phobos182">phobos182</a>
          </li>
      </ul>
    </div>
  </div>

<div class="file-box">
  <div class="file">
    <div class="meta clearfix">
      <div class="info file-name">
          <span>1332 lines (1107 sloc)</span>
          <span class="meta-divider"></span>
        <span>40.34 kb</span>
      </div>
      <div class="actions">
        <div class="button-group">
          <a href="/square/cubism/raw/master/cubism.v1.js" class="minibutton " id="raw-url">Raw</a>
            <a href="/square/cubism/blame/master/cubism.v1.js" class="minibutton js-update-url-with-hash">Blame</a>
          <a href="/square/cubism/commits/master/cubism.v1.js" class="minibutton " rel="nofollow">History</a>
        </div><!-- /.button-group -->

          <a class="octicon-button tooltipped tooltipped-nw"
             href="github-windows://openRepo/https://github.com/square/cubism?branch=master&amp;filepath=cubism.v1.js" aria-label="Open this file in GitHub for Windows">
              <span class="octicon octicon-device-desktop"></span>
          </a>

              <a class="octicon-button tooltipped tooltipped-n js-update-url-with-hash"
                 aria-label="Clicking this button will fork this project so you can edit the file"
                 href="/square/cubism/edit/master/cubism.v1.js"
                 data-method="post" rel="nofollow"><span class="octicon octicon-pencil"></span></a>

            <a class="octicon-button danger tooltipped tooltipped-s"
               href="/square/cubism/delete/master/cubism.v1.js"
               aria-label="Fork this project and delete file"
               data-method="post" data-test-id="delete-blob-file" rel="nofollow">
          <span class="octicon octicon-trashcan"></span>
        </a>
      </div><!-- /.actions -->
    </div>
    
  <div class="blob-wrapper data type-javascript">
      <table class="highlight tab-size-8 js-file-line-container">
      <tr>
        <td id="L1" class="blob-num js-line-number" data-line-number="1"></td>
        <td id="LC1" class="blob-code js-file-line"><span class="p">(</span><span class="kd">function</span><span class="p">(</span><span class="nx">exports</span><span class="p">){</span></td>
      </tr>
      <tr>
        <td id="L2" class="blob-num js-line-number" data-line-number="2"></td>
        <td id="LC2" class="blob-code js-file-line"><span class="kd">var</span> <span class="nx">cubism</span> <span class="o">=</span> <span class="nx">exports</span><span class="p">.</span><span class="nx">cubism</span> <span class="o">=</span> <span class="p">{</span><span class="nx">version</span><span class="o">:</span> <span class="s2">&quot;1.6.0&quot;</span><span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L3" class="blob-num js-line-number" data-line-number="3"></td>
        <td id="LC3" class="blob-code js-file-line"><span class="kd">var</span> <span class="nx">cubism_id</span> <span class="o">=</span> <span class="mi">0</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L4" class="blob-num js-line-number" data-line-number="4"></td>
        <td id="LC4" class="blob-code js-file-line"><span class="kd">function</span> <span class="nx">cubism_identity</span><span class="p">(</span><span class="nx">d</span><span class="p">)</span> <span class="p">{</span> <span class="k">return</span> <span class="nx">d</span><span class="p">;</span> <span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L5" class="blob-num js-line-number" data-line-number="5"></td>
        <td id="LC5" class="blob-code js-file-line"><span class="nx">cubism</span><span class="p">.</span><span class="nx">option</span> <span class="o">=</span> <span class="kd">function</span><span class="p">(</span><span class="nx">name</span><span class="p">,</span> <span class="nx">defaultValue</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L6" class="blob-num js-line-number" data-line-number="6"></td>
        <td id="LC6" class="blob-code js-file-line">  <span class="kd">var</span> <span class="nx">values</span> <span class="o">=</span> <span class="nx">cubism</span><span class="p">.</span><span class="nx">options</span><span class="p">(</span><span class="nx">name</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L7" class="blob-num js-line-number" data-line-number="7"></td>
        <td id="LC7" class="blob-code js-file-line">  <span class="k">return</span> <span class="nx">values</span><span class="p">.</span><span class="nx">length</span> <span class="o">?</span> <span class="nx">values</span><span class="p">[</span><span class="mi">0</span><span class="p">]</span> <span class="o">:</span> <span class="nx">defaultValue</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L8" class="blob-num js-line-number" data-line-number="8"></td>
        <td id="LC8" class="blob-code js-file-line"><span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L9" class="blob-num js-line-number" data-line-number="9"></td>
        <td id="LC9" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L10" class="blob-num js-line-number" data-line-number="10"></td>
        <td id="LC10" class="blob-code js-file-line"><span class="nx">cubism</span><span class="p">.</span><span class="nx">options</span> <span class="o">=</span> <span class="kd">function</span><span class="p">(</span><span class="nx">name</span><span class="p">,</span> <span class="nx">defaultValues</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L11" class="blob-num js-line-number" data-line-number="11"></td>
        <td id="LC11" class="blob-code js-file-line">  <span class="kd">var</span> <span class="nx">options</span> <span class="o">=</span> <span class="nx">location</span><span class="p">.</span><span class="nx">search</span><span class="p">.</span><span class="nx">substring</span><span class="p">(</span><span class="mi">1</span><span class="p">).</span><span class="nx">split</span><span class="p">(</span><span class="s2">&quot;&amp;&quot;</span><span class="p">),</span></td>
      </tr>
      <tr>
        <td id="L12" class="blob-num js-line-number" data-line-number="12"></td>
        <td id="LC12" class="blob-code js-file-line">      <span class="nx">values</span> <span class="o">=</span> <span class="p">[],</span></td>
      </tr>
      <tr>
        <td id="L13" class="blob-num js-line-number" data-line-number="13"></td>
        <td id="LC13" class="blob-code js-file-line">      <span class="nx">i</span> <span class="o">=</span> <span class="o">-</span><span class="mi">1</span><span class="p">,</span></td>
      </tr>
      <tr>
        <td id="L14" class="blob-num js-line-number" data-line-number="14"></td>
        <td id="LC14" class="blob-code js-file-line">      <span class="nx">n</span> <span class="o">=</span> <span class="nx">options</span><span class="p">.</span><span class="nx">length</span><span class="p">,</span></td>
      </tr>
      <tr>
        <td id="L15" class="blob-num js-line-number" data-line-number="15"></td>
        <td id="LC15" class="blob-code js-file-line">      <span class="nx">o</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L16" class="blob-num js-line-number" data-line-number="16"></td>
        <td id="LC16" class="blob-code js-file-line">  <span class="k">while</span> <span class="p">(</span><span class="o">++</span><span class="nx">i</span> <span class="o">&lt;</span> <span class="nx">n</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L17" class="blob-num js-line-number" data-line-number="17"></td>
        <td id="LC17" class="blob-code js-file-line">    <span class="k">if</span> <span class="p">((</span><span class="nx">o</span> <span class="o">=</span> <span class="nx">options</span><span class="p">[</span><span class="nx">i</span><span class="p">].</span><span class="nx">split</span><span class="p">(</span><span class="s2">&quot;=&quot;</span><span class="p">))[</span><span class="mi">0</span><span class="p">]</span> <span class="o">==</span> <span class="nx">name</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L18" class="blob-num js-line-number" data-line-number="18"></td>
        <td id="LC18" class="blob-code js-file-line">      <span class="nx">values</span><span class="p">.</span><span class="nx">push</span><span class="p">(</span><span class="nb">decodeURIComponent</span><span class="p">(</span><span class="nx">o</span><span class="p">[</span><span class="mi">1</span><span class="p">]));</span></td>
      </tr>
      <tr>
        <td id="L19" class="blob-num js-line-number" data-line-number="19"></td>
        <td id="LC19" class="blob-code js-file-line">    <span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L20" class="blob-num js-line-number" data-line-number="20"></td>
        <td id="LC20" class="blob-code js-file-line">  <span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L21" class="blob-num js-line-number" data-line-number="21"></td>
        <td id="LC21" class="blob-code js-file-line">  <span class="k">return</span> <span class="nx">values</span><span class="p">.</span><span class="nx">length</span> <span class="o">||</span> <span class="nx">arguments</span><span class="p">.</span><span class="nx">length</span> <span class="o">&lt;</span> <span class="mi">2</span> <span class="o">?</span> <span class="nx">values</span> <span class="o">:</span> <span class="nx">defaultValues</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L22" class="blob-num js-line-number" data-line-number="22"></td>
        <td id="LC22" class="blob-code js-file-line"><span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L23" class="blob-num js-line-number" data-line-number="23"></td>
        <td id="LC23" class="blob-code js-file-line"><span class="nx">cubism</span><span class="p">.</span><span class="nx">context</span> <span class="o">=</span> <span class="kd">function</span><span class="p">()</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L24" class="blob-num js-line-number" data-line-number="24"></td>
        <td id="LC24" class="blob-code js-file-line">  <span class="kd">var</span> <span class="nx">context</span> <span class="o">=</span> <span class="k">new</span> <span class="nx">cubism_context</span><span class="p">,</span></td>
      </tr>
      <tr>
        <td id="L25" class="blob-num js-line-number" data-line-number="25"></td>
        <td id="LC25" class="blob-code js-file-line">      <span class="nx">step</span> <span class="o">=</span> <span class="mi">1</span><span class="nx">e4</span><span class="p">,</span> <span class="c1">// ten seconds, in milliseconds</span></td>
      </tr>
      <tr>
        <td id="L26" class="blob-num js-line-number" data-line-number="26"></td>
        <td id="LC26" class="blob-code js-file-line">      <span class="nx">size</span> <span class="o">=</span> <span class="mi">1440</span><span class="p">,</span> <span class="c1">// four hours at ten seconds, in pixels</span></td>
      </tr>
      <tr>
        <td id="L27" class="blob-num js-line-number" data-line-number="27"></td>
        <td id="LC27" class="blob-code js-file-line">      <span class="nx">start0</span><span class="p">,</span> <span class="nx">stop0</span><span class="p">,</span> <span class="c1">// the start and stop for the previous change event</span></td>
      </tr>
      <tr>
        <td id="L28" class="blob-num js-line-number" data-line-number="28"></td>
        <td id="LC28" class="blob-code js-file-line">      <span class="nx">start1</span><span class="p">,</span> <span class="nx">stop1</span><span class="p">,</span> <span class="c1">// the start and stop for the next prepare event</span></td>
      </tr>
      <tr>
        <td id="L29" class="blob-num js-line-number" data-line-number="29"></td>
        <td id="LC29" class="blob-code js-file-line">      <span class="nx">serverDelay</span> <span class="o">=</span> <span class="mi">5</span><span class="nx">e3</span><span class="p">,</span></td>
      </tr>
      <tr>
        <td id="L30" class="blob-num js-line-number" data-line-number="30"></td>
        <td id="LC30" class="blob-code js-file-line">      <span class="nx">clientDelay</span> <span class="o">=</span> <span class="mi">5</span><span class="nx">e3</span><span class="p">,</span></td>
      </tr>
      <tr>
        <td id="L31" class="blob-num js-line-number" data-line-number="31"></td>
        <td id="LC31" class="blob-code js-file-line">      <span class="nx">event</span> <span class="o">=</span> <span class="nx">d3</span><span class="p">.</span><span class="nx">dispatch</span><span class="p">(</span><span class="s2">&quot;prepare&quot;</span><span class="p">,</span> <span class="s2">&quot;beforechange&quot;</span><span class="p">,</span> <span class="s2">&quot;change&quot;</span><span class="p">,</span> <span class="s2">&quot;focus&quot;</span><span class="p">),</span></td>
      </tr>
      <tr>
        <td id="L32" class="blob-num js-line-number" data-line-number="32"></td>
        <td id="LC32" class="blob-code js-file-line">      <span class="nx">scale</span> <span class="o">=</span> <span class="nx">context</span><span class="p">.</span><span class="nx">scale</span> <span class="o">=</span> <span class="nx">d3</span><span class="p">.</span><span class="nx">time</span><span class="p">.</span><span class="nx">scale</span><span class="p">().</span><span class="nx">range</span><span class="p">([</span><span class="mi">0</span><span class="p">,</span> <span class="nx">size</span><span class="p">]),</span></td>
      </tr>
      <tr>
        <td id="L33" class="blob-num js-line-number" data-line-number="33"></td>
        <td id="LC33" class="blob-code js-file-line">      <span class="nx">timeout</span><span class="p">,</span></td>
      </tr>
      <tr>
        <td id="L34" class="blob-num js-line-number" data-line-number="34"></td>
        <td id="LC34" class="blob-code js-file-line">      <span class="nx">focus</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L35" class="blob-num js-line-number" data-line-number="35"></td>
        <td id="LC35" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L36" class="blob-num js-line-number" data-line-number="36"></td>
        <td id="LC36" class="blob-code js-file-line">  <span class="kd">function</span> <span class="nx">update</span><span class="p">()</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L37" class="blob-num js-line-number" data-line-number="37"></td>
        <td id="LC37" class="blob-code js-file-line">    <span class="kd">var</span> <span class="nx">now</span> <span class="o">=</span> <span class="nb">Date</span><span class="p">.</span><span class="nx">now</span><span class="p">();</span></td>
      </tr>
      <tr>
        <td id="L38" class="blob-num js-line-number" data-line-number="38"></td>
        <td id="LC38" class="blob-code js-file-line">    <span class="nx">stop0</span> <span class="o">=</span> <span class="k">new</span> <span class="nb">Date</span><span class="p">(</span><span class="nb">Math</span><span class="p">.</span><span class="nx">floor</span><span class="p">((</span><span class="nx">now</span> <span class="o">-</span> <span class="nx">serverDelay</span> <span class="o">-</span> <span class="nx">clientDelay</span><span class="p">)</span> <span class="o">/</span> <span class="nx">step</span><span class="p">)</span> <span class="o">*</span> <span class="nx">step</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L39" class="blob-num js-line-number" data-line-number="39"></td>
        <td id="LC39" class="blob-code js-file-line">    <span class="nx">start0</span> <span class="o">=</span> <span class="k">new</span> <span class="nb">Date</span><span class="p">(</span><span class="nx">stop0</span> <span class="o">-</span> <span class="nx">size</span> <span class="o">*</span> <span class="nx">step</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L40" class="blob-num js-line-number" data-line-number="40"></td>
        <td id="LC40" class="blob-code js-file-line">    <span class="nx">stop1</span> <span class="o">=</span> <span class="k">new</span> <span class="nb">Date</span><span class="p">(</span><span class="nb">Math</span><span class="p">.</span><span class="nx">floor</span><span class="p">((</span><span class="nx">now</span> <span class="o">-</span> <span class="nx">serverDelay</span><span class="p">)</span> <span class="o">/</span> <span class="nx">step</span><span class="p">)</span> <span class="o">*</span> <span class="nx">step</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L41" class="blob-num js-line-number" data-line-number="41"></td>
        <td id="LC41" class="blob-code js-file-line">    <span class="nx">start1</span> <span class="o">=</span> <span class="k">new</span> <span class="nb">Date</span><span class="p">(</span><span class="nx">stop1</span> <span class="o">-</span> <span class="nx">size</span> <span class="o">*</span> <span class="nx">step</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L42" class="blob-num js-line-number" data-line-number="42"></td>
        <td id="LC42" class="blob-code js-file-line">    <span class="nx">scale</span><span class="p">.</span><span class="nx">domain</span><span class="p">([</span><span class="nx">start0</span><span class="p">,</span> <span class="nx">stop0</span><span class="p">]);</span></td>
      </tr>
      <tr>
        <td id="L43" class="blob-num js-line-number" data-line-number="43"></td>
        <td id="LC43" class="blob-code js-file-line">    <span class="k">return</span> <span class="nx">context</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L44" class="blob-num js-line-number" data-line-number="44"></td>
        <td id="LC44" class="blob-code js-file-line">  <span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L45" class="blob-num js-line-number" data-line-number="45"></td>
        <td id="LC45" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L46" class="blob-num js-line-number" data-line-number="46"></td>
        <td id="LC46" class="blob-code js-file-line">  <span class="nx">context</span><span class="p">.</span><span class="nx">start</span> <span class="o">=</span> <span class="kd">function</span><span class="p">()</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L47" class="blob-num js-line-number" data-line-number="47"></td>
        <td id="LC47" class="blob-code js-file-line">    <span class="k">if</span> <span class="p">(</span><span class="nx">timeout</span><span class="p">)</span> <span class="nx">clearTimeout</span><span class="p">(</span><span class="nx">timeout</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L48" class="blob-num js-line-number" data-line-number="48"></td>
        <td id="LC48" class="blob-code js-file-line">    <span class="kd">var</span> <span class="nx">delay</span> <span class="o">=</span> <span class="o">+</span><span class="nx">stop1</span> <span class="o">+</span> <span class="nx">serverDelay</span> <span class="o">-</span> <span class="nb">Date</span><span class="p">.</span><span class="nx">now</span><span class="p">();</span></td>
      </tr>
      <tr>
        <td id="L49" class="blob-num js-line-number" data-line-number="49"></td>
        <td id="LC49" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L50" class="blob-num js-line-number" data-line-number="50"></td>
        <td id="LC50" class="blob-code js-file-line">    <span class="c1">// If we&#39;re too late for the first prepare event, skip it.</span></td>
      </tr>
      <tr>
        <td id="L51" class="blob-num js-line-number" data-line-number="51"></td>
        <td id="LC51" class="blob-code js-file-line">    <span class="k">if</span> <span class="p">(</span><span class="nx">delay</span> <span class="o">&lt;</span> <span class="nx">clientDelay</span><span class="p">)</span> <span class="nx">delay</span> <span class="o">+=</span> <span class="nx">step</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L52" class="blob-num js-line-number" data-line-number="52"></td>
        <td id="LC52" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L53" class="blob-num js-line-number" data-line-number="53"></td>
        <td id="LC53" class="blob-code js-file-line">    <span class="nx">timeout</span> <span class="o">=</span> <span class="nx">setTimeout</span><span class="p">(</span><span class="kd">function</span> <span class="nx">prepare</span><span class="p">()</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L54" class="blob-num js-line-number" data-line-number="54"></td>
        <td id="LC54" class="blob-code js-file-line">      <span class="nx">stop1</span> <span class="o">=</span> <span class="k">new</span> <span class="nb">Date</span><span class="p">(</span><span class="nb">Math</span><span class="p">.</span><span class="nx">floor</span><span class="p">((</span><span class="nb">Date</span><span class="p">.</span><span class="nx">now</span><span class="p">()</span> <span class="o">-</span> <span class="nx">serverDelay</span><span class="p">)</span> <span class="o">/</span> <span class="nx">step</span><span class="p">)</span> <span class="o">*</span> <span class="nx">step</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L55" class="blob-num js-line-number" data-line-number="55"></td>
        <td id="LC55" class="blob-code js-file-line">      <span class="nx">start1</span> <span class="o">=</span> <span class="k">new</span> <span class="nb">Date</span><span class="p">(</span><span class="nx">stop1</span> <span class="o">-</span> <span class="nx">size</span> <span class="o">*</span> <span class="nx">step</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L56" class="blob-num js-line-number" data-line-number="56"></td>
        <td id="LC56" class="blob-code js-file-line">      <span class="nx">event</span><span class="p">.</span><span class="nx">prepare</span><span class="p">.</span><span class="nx">call</span><span class="p">(</span><span class="nx">context</span><span class="p">,</span> <span class="nx">start1</span><span class="p">,</span> <span class="nx">stop1</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L57" class="blob-num js-line-number" data-line-number="57"></td>
        <td id="LC57" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L58" class="blob-num js-line-number" data-line-number="58"></td>
        <td id="LC58" class="blob-code js-file-line">      <span class="nx">setTimeout</span><span class="p">(</span><span class="kd">function</span><span class="p">()</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L59" class="blob-num js-line-number" data-line-number="59"></td>
        <td id="LC59" class="blob-code js-file-line">        <span class="nx">scale</span><span class="p">.</span><span class="nx">domain</span><span class="p">([</span><span class="nx">start0</span> <span class="o">=</span> <span class="nx">start1</span><span class="p">,</span> <span class="nx">stop0</span> <span class="o">=</span> <span class="nx">stop1</span><span class="p">]);</span></td>
      </tr>
      <tr>
        <td id="L60" class="blob-num js-line-number" data-line-number="60"></td>
        <td id="LC60" class="blob-code js-file-line">        <span class="nx">event</span><span class="p">.</span><span class="nx">beforechange</span><span class="p">.</span><span class="nx">call</span><span class="p">(</span><span class="nx">context</span><span class="p">,</span> <span class="nx">start1</span><span class="p">,</span> <span class="nx">stop1</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L61" class="blob-num js-line-number" data-line-number="61"></td>
        <td id="LC61" class="blob-code js-file-line">        <span class="nx">event</span><span class="p">.</span><span class="nx">change</span><span class="p">.</span><span class="nx">call</span><span class="p">(</span><span class="nx">context</span><span class="p">,</span> <span class="nx">start1</span><span class="p">,</span> <span class="nx">stop1</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L62" class="blob-num js-line-number" data-line-number="62"></td>
        <td id="LC62" class="blob-code js-file-line">        <span class="nx">event</span><span class="p">.</span><span class="nx">focus</span><span class="p">.</span><span class="nx">call</span><span class="p">(</span><span class="nx">context</span><span class="p">,</span> <span class="nx">focus</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L63" class="blob-num js-line-number" data-line-number="63"></td>
        <td id="LC63" class="blob-code js-file-line">      <span class="p">},</span> <span class="nx">clientDelay</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L64" class="blob-num js-line-number" data-line-number="64"></td>
        <td id="LC64" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L65" class="blob-num js-line-number" data-line-number="65"></td>
        <td id="LC65" class="blob-code js-file-line">      <span class="nx">timeout</span> <span class="o">=</span> <span class="nx">setTimeout</span><span class="p">(</span><span class="nx">prepare</span><span class="p">,</span> <span class="nx">step</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L66" class="blob-num js-line-number" data-line-number="66"></td>
        <td id="LC66" class="blob-code js-file-line">    <span class="p">},</span> <span class="nx">delay</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L67" class="blob-num js-line-number" data-line-number="67"></td>
        <td id="LC67" class="blob-code js-file-line">    <span class="k">return</span> <span class="nx">context</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L68" class="blob-num js-line-number" data-line-number="68"></td>
        <td id="LC68" class="blob-code js-file-line">  <span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L69" class="blob-num js-line-number" data-line-number="69"></td>
        <td id="LC69" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L70" class="blob-num js-line-number" data-line-number="70"></td>
        <td id="LC70" class="blob-code js-file-line">  <span class="nx">context</span><span class="p">.</span><span class="nx">stop</span> <span class="o">=</span> <span class="kd">function</span><span class="p">()</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L71" class="blob-num js-line-number" data-line-number="71"></td>
        <td id="LC71" class="blob-code js-file-line">    <span class="nx">timeout</span> <span class="o">=</span> <span class="nx">clearTimeout</span><span class="p">(</span><span class="nx">timeout</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L72" class="blob-num js-line-number" data-line-number="72"></td>
        <td id="LC72" class="blob-code js-file-line">    <span class="k">return</span> <span class="nx">context</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L73" class="blob-num js-line-number" data-line-number="73"></td>
        <td id="LC73" class="blob-code js-file-line">  <span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L74" class="blob-num js-line-number" data-line-number="74"></td>
        <td id="LC74" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L75" class="blob-num js-line-number" data-line-number="75"></td>
        <td id="LC75" class="blob-code js-file-line">  <span class="nx">timeout</span> <span class="o">=</span> <span class="nx">setTimeout</span><span class="p">(</span><span class="nx">context</span><span class="p">.</span><span class="nx">start</span><span class="p">,</span> <span class="mi">10</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L76" class="blob-num js-line-number" data-line-number="76"></td>
        <td id="LC76" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L77" class="blob-num js-line-number" data-line-number="77"></td>
        <td id="LC77" class="blob-code js-file-line">  <span class="c1">// Set or get the step interval in milliseconds.</span></td>
      </tr>
      <tr>
        <td id="L78" class="blob-num js-line-number" data-line-number="78"></td>
        <td id="LC78" class="blob-code js-file-line">  <span class="c1">// Defaults to ten seconds.</span></td>
      </tr>
      <tr>
        <td id="L79" class="blob-num js-line-number" data-line-number="79"></td>
        <td id="LC79" class="blob-code js-file-line">  <span class="nx">context</span><span class="p">.</span><span class="nx">step</span> <span class="o">=</span> <span class="kd">function</span><span class="p">(</span><span class="nx">_</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L80" class="blob-num js-line-number" data-line-number="80"></td>
        <td id="LC80" class="blob-code js-file-line">    <span class="k">if</span> <span class="p">(</span><span class="o">!</span><span class="nx">arguments</span><span class="p">.</span><span class="nx">length</span><span class="p">)</span> <span class="k">return</span> <span class="nx">step</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L81" class="blob-num js-line-number" data-line-number="81"></td>
        <td id="LC81" class="blob-code js-file-line">    <span class="nx">step</span> <span class="o">=</span> <span class="o">+</span><span class="nx">_</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L82" class="blob-num js-line-number" data-line-number="82"></td>
        <td id="LC82" class="blob-code js-file-line">    <span class="k">return</span> <span class="nx">update</span><span class="p">();</span></td>
      </tr>
      <tr>
        <td id="L83" class="blob-num js-line-number" data-line-number="83"></td>
        <td id="LC83" class="blob-code js-file-line">  <span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L84" class="blob-num js-line-number" data-line-number="84"></td>
        <td id="LC84" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L85" class="blob-num js-line-number" data-line-number="85"></td>
        <td id="LC85" class="blob-code js-file-line">  <span class="c1">// Set or get the context size (the count of metric values).</span></td>
      </tr>
      <tr>
        <td id="L86" class="blob-num js-line-number" data-line-number="86"></td>
        <td id="LC86" class="blob-code js-file-line">  <span class="c1">// Defaults to 1440 (four hours at ten seconds).</span></td>
      </tr>
      <tr>
        <td id="L87" class="blob-num js-line-number" data-line-number="87"></td>
        <td id="LC87" class="blob-code js-file-line">  <span class="nx">context</span><span class="p">.</span><span class="nx">size</span> <span class="o">=</span> <span class="kd">function</span><span class="p">(</span><span class="nx">_</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L88" class="blob-num js-line-number" data-line-number="88"></td>
        <td id="LC88" class="blob-code js-file-line">    <span class="k">if</span> <span class="p">(</span><span class="o">!</span><span class="nx">arguments</span><span class="p">.</span><span class="nx">length</span><span class="p">)</span> <span class="k">return</span> <span class="nx">size</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L89" class="blob-num js-line-number" data-line-number="89"></td>
        <td id="LC89" class="blob-code js-file-line">    <span class="nx">scale</span><span class="p">.</span><span class="nx">range</span><span class="p">([</span><span class="mi">0</span><span class="p">,</span> <span class="nx">size</span> <span class="o">=</span> <span class="o">+</span><span class="nx">_</span><span class="p">]);</span></td>
      </tr>
      <tr>
        <td id="L90" class="blob-num js-line-number" data-line-number="90"></td>
        <td id="LC90" class="blob-code js-file-line">    <span class="k">return</span> <span class="nx">update</span><span class="p">();</span></td>
      </tr>
      <tr>
        <td id="L91" class="blob-num js-line-number" data-line-number="91"></td>
        <td id="LC91" class="blob-code js-file-line">  <span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L92" class="blob-num js-line-number" data-line-number="92"></td>
        <td id="LC92" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L93" class="blob-num js-line-number" data-line-number="93"></td>
        <td id="LC93" class="blob-code js-file-line">  <span class="c1">// The server delay is the amount of time we wait for the server to compute a</span></td>
      </tr>
      <tr>
        <td id="L94" class="blob-num js-line-number" data-line-number="94"></td>
        <td id="LC94" class="blob-code js-file-line">  <span class="c1">// metric. This delay may result from clock skew or from delays collecting</span></td>
      </tr>
      <tr>
        <td id="L95" class="blob-num js-line-number" data-line-number="95"></td>
        <td id="LC95" class="blob-code js-file-line">  <span class="c1">// metrics from various hosts. Defaults to 4 seconds.</span></td>
      </tr>
      <tr>
        <td id="L96" class="blob-num js-line-number" data-line-number="96"></td>
        <td id="LC96" class="blob-code js-file-line">  <span class="nx">context</span><span class="p">.</span><span class="nx">serverDelay</span> <span class="o">=</span> <span class="kd">function</span><span class="p">(</span><span class="nx">_</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L97" class="blob-num js-line-number" data-line-number="97"></td>
        <td id="LC97" class="blob-code js-file-line">    <span class="k">if</span> <span class="p">(</span><span class="o">!</span><span class="nx">arguments</span><span class="p">.</span><span class="nx">length</span><span class="p">)</span> <span class="k">return</span> <span class="nx">serverDelay</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L98" class="blob-num js-line-number" data-line-number="98"></td>
        <td id="LC98" class="blob-code js-file-line">    <span class="nx">serverDelay</span> <span class="o">=</span> <span class="o">+</span><span class="nx">_</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L99" class="blob-num js-line-number" data-line-number="99"></td>
        <td id="LC99" class="blob-code js-file-line">    <span class="k">return</span> <span class="nx">update</span><span class="p">();</span></td>
      </tr>
      <tr>
        <td id="L100" class="blob-num js-line-number" data-line-number="100"></td>
        <td id="LC100" class="blob-code js-file-line">  <span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L101" class="blob-num js-line-number" data-line-number="101"></td>
        <td id="LC101" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L102" class="blob-num js-line-number" data-line-number="102"></td>
        <td id="LC102" class="blob-code js-file-line">  <span class="c1">// The client delay is the amount of additional time we wait to fetch those</span></td>
      </tr>
      <tr>
        <td id="L103" class="blob-num js-line-number" data-line-number="103"></td>
        <td id="LC103" class="blob-code js-file-line">  <span class="c1">// metrics from the server. The client and server delay combined represent the</span></td>
      </tr>
      <tr>
        <td id="L104" class="blob-num js-line-number" data-line-number="104"></td>
        <td id="LC104" class="blob-code js-file-line">  <span class="c1">// age of the most recent displayed metric. Defaults to 1 second.</span></td>
      </tr>
      <tr>
        <td id="L105" class="blob-num js-line-number" data-line-number="105"></td>
        <td id="LC105" class="blob-code js-file-line">  <span class="nx">context</span><span class="p">.</span><span class="nx">clientDelay</span> <span class="o">=</span> <span class="kd">function</span><span class="p">(</span><span class="nx">_</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L106" class="blob-num js-line-number" data-line-number="106"></td>
        <td id="LC106" class="blob-code js-file-line">    <span class="k">if</span> <span class="p">(</span><span class="o">!</span><span class="nx">arguments</span><span class="p">.</span><span class="nx">length</span><span class="p">)</span> <span class="k">return</span> <span class="nx">clientDelay</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L107" class="blob-num js-line-number" data-line-number="107"></td>
        <td id="LC107" class="blob-code js-file-line">    <span class="nx">clientDelay</span> <span class="o">=</span> <span class="o">+</span><span class="nx">_</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L108" class="blob-num js-line-number" data-line-number="108"></td>
        <td id="LC108" class="blob-code js-file-line">    <span class="k">return</span> <span class="nx">update</span><span class="p">();</span></td>
      </tr>
      <tr>
        <td id="L109" class="blob-num js-line-number" data-line-number="109"></td>
        <td id="LC109" class="blob-code js-file-line">  <span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L110" class="blob-num js-line-number" data-line-number="110"></td>
        <td id="LC110" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L111" class="blob-num js-line-number" data-line-number="111"></td>
        <td id="LC111" class="blob-code js-file-line">  <span class="c1">// Sets the focus to the specified index, and dispatches a &quot;focus&quot; event.</span></td>
      </tr>
      <tr>
        <td id="L112" class="blob-num js-line-number" data-line-number="112"></td>
        <td id="LC112" class="blob-code js-file-line">  <span class="nx">context</span><span class="p">.</span><span class="nx">focus</span> <span class="o">=</span> <span class="kd">function</span><span class="p">(</span><span class="nx">i</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L113" class="blob-num js-line-number" data-line-number="113"></td>
        <td id="LC113" class="blob-code js-file-line">    <span class="nx">event</span><span class="p">.</span><span class="nx">focus</span><span class="p">.</span><span class="nx">call</span><span class="p">(</span><span class="nx">context</span><span class="p">,</span> <span class="nx">focus</span> <span class="o">=</span> <span class="nx">i</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L114" class="blob-num js-line-number" data-line-number="114"></td>
        <td id="LC114" class="blob-code js-file-line">    <span class="k">return</span> <span class="nx">context</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L115" class="blob-num js-line-number" data-line-number="115"></td>
        <td id="LC115" class="blob-code js-file-line">  <span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L116" class="blob-num js-line-number" data-line-number="116"></td>
        <td id="LC116" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L117" class="blob-num js-line-number" data-line-number="117"></td>
        <td id="LC117" class="blob-code js-file-line">  <span class="c1">// Add, remove or get listeners for events.</span></td>
      </tr>
      <tr>
        <td id="L118" class="blob-num js-line-number" data-line-number="118"></td>
        <td id="LC118" class="blob-code js-file-line">  <span class="nx">context</span><span class="p">.</span><span class="nx">on</span> <span class="o">=</span> <span class="kd">function</span><span class="p">(</span><span class="nx">type</span><span class="p">,</span> <span class="nx">listener</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L119" class="blob-num js-line-number" data-line-number="119"></td>
        <td id="LC119" class="blob-code js-file-line">    <span class="k">if</span> <span class="p">(</span><span class="nx">arguments</span><span class="p">.</span><span class="nx">length</span> <span class="o">&lt;</span> <span class="mi">2</span><span class="p">)</span> <span class="k">return</span> <span class="nx">event</span><span class="p">.</span><span class="nx">on</span><span class="p">(</span><span class="nx">type</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L120" class="blob-num js-line-number" data-line-number="120"></td>
        <td id="LC120" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L121" class="blob-num js-line-number" data-line-number="121"></td>
        <td id="LC121" class="blob-code js-file-line">    <span class="nx">event</span><span class="p">.</span><span class="nx">on</span><span class="p">(</span><span class="nx">type</span><span class="p">,</span> <span class="nx">listener</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L122" class="blob-num js-line-number" data-line-number="122"></td>
        <td id="LC122" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L123" class="blob-num js-line-number" data-line-number="123"></td>
        <td id="LC123" class="blob-code js-file-line">    <span class="c1">// Notify the listener of the current start and stop time, as appropriate.</span></td>
      </tr>
      <tr>
        <td id="L124" class="blob-num js-line-number" data-line-number="124"></td>
        <td id="LC124" class="blob-code js-file-line">    <span class="c1">// This way, metrics can make requests for data immediately,</span></td>
      </tr>
      <tr>
        <td id="L125" class="blob-num js-line-number" data-line-number="125"></td>
        <td id="LC125" class="blob-code js-file-line">    <span class="c1">// and likewise the axis can display itself synchronously.</span></td>
      </tr>
      <tr>
        <td id="L126" class="blob-num js-line-number" data-line-number="126"></td>
        <td id="LC126" class="blob-code js-file-line">    <span class="k">if</span> <span class="p">(</span><span class="nx">listener</span> <span class="o">!=</span> <span class="kc">null</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L127" class="blob-num js-line-number" data-line-number="127"></td>
        <td id="LC127" class="blob-code js-file-line">      <span class="k">if</span> <span class="p">(</span><span class="sr">/^prepare(\.|$)/</span><span class="p">.</span><span class="nx">test</span><span class="p">(</span><span class="nx">type</span><span class="p">))</span> <span class="nx">listener</span><span class="p">.</span><span class="nx">call</span><span class="p">(</span><span class="nx">context</span><span class="p">,</span> <span class="nx">start1</span><span class="p">,</span> <span class="nx">stop1</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L128" class="blob-num js-line-number" data-line-number="128"></td>
        <td id="LC128" class="blob-code js-file-line">      <span class="k">if</span> <span class="p">(</span><span class="sr">/^beforechange(\.|$)/</span><span class="p">.</span><span class="nx">test</span><span class="p">(</span><span class="nx">type</span><span class="p">))</span> <span class="nx">listener</span><span class="p">.</span><span class="nx">call</span><span class="p">(</span><span class="nx">context</span><span class="p">,</span> <span class="nx">start0</span><span class="p">,</span> <span class="nx">stop0</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L129" class="blob-num js-line-number" data-line-number="129"></td>
        <td id="LC129" class="blob-code js-file-line">      <span class="k">if</span> <span class="p">(</span><span class="sr">/^change(\.|$)/</span><span class="p">.</span><span class="nx">test</span><span class="p">(</span><span class="nx">type</span><span class="p">))</span> <span class="nx">listener</span><span class="p">.</span><span class="nx">call</span><span class="p">(</span><span class="nx">context</span><span class="p">,</span> <span class="nx">start0</span><span class="p">,</span> <span class="nx">stop0</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L130" class="blob-num js-line-number" data-line-number="130"></td>
        <td id="LC130" class="blob-code js-file-line">      <span class="k">if</span> <span class="p">(</span><span class="sr">/^focus(\.|$)/</span><span class="p">.</span><span class="nx">test</span><span class="p">(</span><span class="nx">type</span><span class="p">))</span> <span class="nx">listener</span><span class="p">.</span><span class="nx">call</span><span class="p">(</span><span class="nx">context</span><span class="p">,</span> <span class="nx">focus</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L131" class="blob-num js-line-number" data-line-number="131"></td>
        <td id="LC131" class="blob-code js-file-line">    <span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L132" class="blob-num js-line-number" data-line-number="132"></td>
        <td id="LC132" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L133" class="blob-num js-line-number" data-line-number="133"></td>
        <td id="LC133" class="blob-code js-file-line">    <span class="k">return</span> <span class="nx">context</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L134" class="blob-num js-line-number" data-line-number="134"></td>
        <td id="LC134" class="blob-code js-file-line">  <span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L135" class="blob-num js-line-number" data-line-number="135"></td>
        <td id="LC135" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L136" class="blob-num js-line-number" data-line-number="136"></td>
        <td id="LC136" class="blob-code js-file-line">  <span class="nx">d3</span><span class="p">.</span><span class="nx">select</span><span class="p">(</span><span class="nb">window</span><span class="p">).</span><span class="nx">on</span><span class="p">(</span><span class="s2">&quot;keydown.context-&quot;</span> <span class="o">+</span> <span class="o">++</span><span class="nx">cubism_id</span><span class="p">,</span> <span class="kd">function</span><span class="p">()</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L137" class="blob-num js-line-number" data-line-number="137"></td>
        <td id="LC137" class="blob-code js-file-line">    <span class="k">switch</span> <span class="p">(</span><span class="o">!</span><span class="nx">d3</span><span class="p">.</span><span class="nx">event</span><span class="p">.</span><span class="nx">metaKey</span> <span class="o">&amp;&amp;</span> <span class="nx">d3</span><span class="p">.</span><span class="nx">event</span><span class="p">.</span><span class="nx">keyCode</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L138" class="blob-num js-line-number" data-line-number="138"></td>
        <td id="LC138" class="blob-code js-file-line">      <span class="k">case</span> <span class="mi">37</span><span class="o">:</span> <span class="c1">// left</span></td>
      </tr>
      <tr>
        <td id="L139" class="blob-num js-line-number" data-line-number="139"></td>
        <td id="LC139" class="blob-code js-file-line">        <span class="k">if</span> <span class="p">(</span><span class="nx">focus</span> <span class="o">==</span> <span class="kc">null</span><span class="p">)</span> <span class="nx">focus</span> <span class="o">=</span> <span class="nx">size</span> <span class="o">-</span> <span class="mi">1</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L140" class="blob-num js-line-number" data-line-number="140"></td>
        <td id="LC140" class="blob-code js-file-line">        <span class="k">if</span> <span class="p">(</span><span class="nx">focus</span> <span class="o">&gt;</span> <span class="mi">0</span><span class="p">)</span> <span class="nx">context</span><span class="p">.</span><span class="nx">focus</span><span class="p">(</span><span class="o">--</span><span class="nx">focus</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L141" class="blob-num js-line-number" data-line-number="141"></td>
        <td id="LC141" class="blob-code js-file-line">        <span class="k">break</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L142" class="blob-num js-line-number" data-line-number="142"></td>
        <td id="LC142" class="blob-code js-file-line">      <span class="k">case</span> <span class="mi">39</span><span class="o">:</span> <span class="c1">// right</span></td>
      </tr>
      <tr>
        <td id="L143" class="blob-num js-line-number" data-line-number="143"></td>
        <td id="LC143" class="blob-code js-file-line">        <span class="k">if</span> <span class="p">(</span><span class="nx">focus</span> <span class="o">==</span> <span class="kc">null</span><span class="p">)</span> <span class="nx">focus</span> <span class="o">=</span> <span class="nx">size</span> <span class="o">-</span> <span class="mi">2</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L144" class="blob-num js-line-number" data-line-number="144"></td>
        <td id="LC144" class="blob-code js-file-line">        <span class="k">if</span> <span class="p">(</span><span class="nx">focus</span> <span class="o">&lt;</span> <span class="nx">size</span> <span class="o">-</span> <span class="mi">1</span><span class="p">)</span> <span class="nx">context</span><span class="p">.</span><span class="nx">focus</span><span class="p">(</span><span class="o">++</span><span class="nx">focus</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L145" class="blob-num js-line-number" data-line-number="145"></td>
        <td id="LC145" class="blob-code js-file-line">        <span class="k">break</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L146" class="blob-num js-line-number" data-line-number="146"></td>
        <td id="LC146" class="blob-code js-file-line">      <span class="k">default</span><span class="o">:</span> <span class="k">return</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L147" class="blob-num js-line-number" data-line-number="147"></td>
        <td id="LC147" class="blob-code js-file-line">    <span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L148" class="blob-num js-line-number" data-line-number="148"></td>
        <td id="LC148" class="blob-code js-file-line">    <span class="nx">d3</span><span class="p">.</span><span class="nx">event</span><span class="p">.</span><span class="nx">preventDefault</span><span class="p">();</span></td>
      </tr>
      <tr>
        <td id="L149" class="blob-num js-line-number" data-line-number="149"></td>
        <td id="LC149" class="blob-code js-file-line">  <span class="p">});</span></td>
      </tr>
      <tr>
        <td id="L150" class="blob-num js-line-number" data-line-number="150"></td>
        <td id="LC150" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L151" class="blob-num js-line-number" data-line-number="151"></td>
        <td id="LC151" class="blob-code js-file-line">  <span class="k">return</span> <span class="nx">update</span><span class="p">();</span></td>
      </tr>
      <tr>
        <td id="L152" class="blob-num js-line-number" data-line-number="152"></td>
        <td id="LC152" class="blob-code js-file-line"><span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L153" class="blob-num js-line-number" data-line-number="153"></td>
        <td id="LC153" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L154" class="blob-num js-line-number" data-line-number="154"></td>
        <td id="LC154" class="blob-code js-file-line"><span class="kd">function</span> <span class="nx">cubism_context</span><span class="p">()</span> <span class="p">{}</span></td>
      </tr>
      <tr>
        <td id="L155" class="blob-num js-line-number" data-line-number="155"></td>
        <td id="LC155" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L156" class="blob-num js-line-number" data-line-number="156"></td>
        <td id="LC156" class="blob-code js-file-line"><span class="kd">var</span> <span class="nx">cubism_contextPrototype</span> <span class="o">=</span> <span class="nx">cubism</span><span class="p">.</span><span class="nx">context</span><span class="p">.</span><span class="nx">prototype</span> <span class="o">=</span> <span class="nx">cubism_context</span><span class="p">.</span><span class="nx">prototype</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L157" class="blob-num js-line-number" data-line-number="157"></td>
        <td id="LC157" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L158" class="blob-num js-line-number" data-line-number="158"></td>
        <td id="LC158" class="blob-code js-file-line"><span class="nx">cubism_contextPrototype</span><span class="p">.</span><span class="nx">constant</span> <span class="o">=</span> <span class="kd">function</span><span class="p">(</span><span class="nx">value</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L159" class="blob-num js-line-number" data-line-number="159"></td>
        <td id="LC159" class="blob-code js-file-line">  <span class="k">return</span> <span class="k">new</span> <span class="nx">cubism_metricConstant</span><span class="p">(</span><span class="k">this</span><span class="p">,</span> <span class="o">+</span><span class="nx">value</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L160" class="blob-num js-line-number" data-line-number="160"></td>
        <td id="LC160" class="blob-code js-file-line"><span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L161" class="blob-num js-line-number" data-line-number="161"></td>
        <td id="LC161" class="blob-code js-file-line"><span class="nx">cubism_contextPrototype</span><span class="p">.</span><span class="nx">cube</span> <span class="o">=</span> <span class="kd">function</span><span class="p">(</span><span class="nx">host</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L162" class="blob-num js-line-number" data-line-number="162"></td>
        <td id="LC162" class="blob-code js-file-line">  <span class="k">if</span> <span class="p">(</span><span class="o">!</span><span class="nx">arguments</span><span class="p">.</span><span class="nx">length</span><span class="p">)</span> <span class="nx">host</span> <span class="o">=</span> <span class="s2">&quot;&quot;</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L163" class="blob-num js-line-number" data-line-number="163"></td>
        <td id="LC163" class="blob-code js-file-line">  <span class="kd">var</span> <span class="nx">source</span> <span class="o">=</span> <span class="p">{},</span></td>
      </tr>
      <tr>
        <td id="L164" class="blob-num js-line-number" data-line-number="164"></td>
        <td id="LC164" class="blob-code js-file-line">      <span class="nx">context</span> <span class="o">=</span> <span class="k">this</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L165" class="blob-num js-line-number" data-line-number="165"></td>
        <td id="LC165" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L166" class="blob-num js-line-number" data-line-number="166"></td>
        <td id="LC166" class="blob-code js-file-line">  <span class="nx">source</span><span class="p">.</span><span class="nx">metric</span> <span class="o">=</span> <span class="kd">function</span><span class="p">(</span><span class="nx">expression</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L167" class="blob-num js-line-number" data-line-number="167"></td>
        <td id="LC167" class="blob-code js-file-line">    <span class="k">return</span> <span class="nx">context</span><span class="p">.</span><span class="nx">metric</span><span class="p">(</span><span class="kd">function</span><span class="p">(</span><span class="nx">start</span><span class="p">,</span> <span class="nx">stop</span><span class="p">,</span> <span class="nx">step</span><span class="p">,</span> <span class="nx">callback</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L168" class="blob-num js-line-number" data-line-number="168"></td>
        <td id="LC168" class="blob-code js-file-line">      <span class="nx">d3</span><span class="p">.</span><span class="nx">json</span><span class="p">(</span><span class="nx">host</span> <span class="o">+</span> <span class="s2">&quot;/1.0/metric&quot;</span></td>
      </tr>
      <tr>
        <td id="L169" class="blob-num js-line-number" data-line-number="169"></td>
        <td id="LC169" class="blob-code js-file-line">          <span class="o">+</span> <span class="s2">&quot;?expression=&quot;</span> <span class="o">+</span> <span class="nb">encodeURIComponent</span><span class="p">(</span><span class="nx">expression</span><span class="p">)</span></td>
      </tr>
      <tr>
        <td id="L170" class="blob-num js-line-number" data-line-number="170"></td>
        <td id="LC170" class="blob-code js-file-line">          <span class="o">+</span> <span class="s2">&quot;&amp;start=&quot;</span> <span class="o">+</span> <span class="nx">cubism_cubeFormatDate</span><span class="p">(</span><span class="nx">start</span><span class="p">)</span></td>
      </tr>
      <tr>
        <td id="L171" class="blob-num js-line-number" data-line-number="171"></td>
        <td id="LC171" class="blob-code js-file-line">          <span class="o">+</span> <span class="s2">&quot;&amp;stop=&quot;</span> <span class="o">+</span> <span class="nx">cubism_cubeFormatDate</span><span class="p">(</span><span class="nx">stop</span><span class="p">)</span></td>
      </tr>
      <tr>
        <td id="L172" class="blob-num js-line-number" data-line-number="172"></td>
        <td id="LC172" class="blob-code js-file-line">          <span class="o">+</span> <span class="s2">&quot;&amp;step=&quot;</span> <span class="o">+</span> <span class="nx">step</span><span class="p">,</span> <span class="kd">function</span><span class="p">(</span><span class="nx">data</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L173" class="blob-num js-line-number" data-line-number="173"></td>
        <td id="LC173" class="blob-code js-file-line">        <span class="k">if</span> <span class="p">(</span><span class="o">!</span><span class="nx">data</span><span class="p">)</span> <span class="k">return</span> <span class="nx">callback</span><span class="p">(</span><span class="k">new</span> <span class="nb">Error</span><span class="p">(</span><span class="s2">&quot;unable to load data&quot;</span><span class="p">));</span></td>
      </tr>
      <tr>
        <td id="L174" class="blob-num js-line-number" data-line-number="174"></td>
        <td id="LC174" class="blob-code js-file-line">        <span class="nx">callback</span><span class="p">(</span><span class="kc">null</span><span class="p">,</span> <span class="nx">data</span><span class="p">.</span><span class="nx">map</span><span class="p">(</span><span class="kd">function</span><span class="p">(</span><span class="nx">d</span><span class="p">)</span> <span class="p">{</span> <span class="k">return</span> <span class="nx">d</span><span class="p">.</span><span class="nx">value</span><span class="p">;</span> <span class="p">}));</span></td>
      </tr>
      <tr>
        <td id="L175" class="blob-num js-line-number" data-line-number="175"></td>
        <td id="LC175" class="blob-code js-file-line">      <span class="p">});</span></td>
      </tr>
      <tr>
        <td id="L176" class="blob-num js-line-number" data-line-number="176"></td>
        <td id="LC176" class="blob-code js-file-line">    <span class="p">},</span> <span class="nx">expression</span> <span class="o">+=</span> <span class="s2">&quot;&quot;</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L177" class="blob-num js-line-number" data-line-number="177"></td>
        <td id="LC177" class="blob-code js-file-line">  <span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L178" class="blob-num js-line-number" data-line-number="178"></td>
        <td id="LC178" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L179" class="blob-num js-line-number" data-line-number="179"></td>
        <td id="LC179" class="blob-code js-file-line">  <span class="c1">// Returns the Cube host.</span></td>
      </tr>
      <tr>
        <td id="L180" class="blob-num js-line-number" data-line-number="180"></td>
        <td id="LC180" class="blob-code js-file-line">  <span class="nx">source</span><span class="p">.</span><span class="nx">toString</span> <span class="o">=</span> <span class="kd">function</span><span class="p">()</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L181" class="blob-num js-line-number" data-line-number="181"></td>
        <td id="LC181" class="blob-code js-file-line">    <span class="k">return</span> <span class="nx">host</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L182" class="blob-num js-line-number" data-line-number="182"></td>
        <td id="LC182" class="blob-code js-file-line">  <span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L183" class="blob-num js-line-number" data-line-number="183"></td>
        <td id="LC183" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L184" class="blob-num js-line-number" data-line-number="184"></td>
        <td id="LC184" class="blob-code js-file-line">  <span class="k">return</span> <span class="nx">source</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L185" class="blob-num js-line-number" data-line-number="185"></td>
        <td id="LC185" class="blob-code js-file-line"><span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L186" class="blob-num js-line-number" data-line-number="186"></td>
        <td id="LC186" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L187" class="blob-num js-line-number" data-line-number="187"></td>
        <td id="LC187" class="blob-code js-file-line"><span class="kd">var</span> <span class="nx">cubism_cubeFormatDate</span> <span class="o">=</span> <span class="nx">d3</span><span class="p">.</span><span class="nx">time</span><span class="p">.</span><span class="nx">format</span><span class="p">.</span><span class="nx">iso</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L188" class="blob-num js-line-number" data-line-number="188"></td>
        <td id="LC188" class="blob-code js-file-line"><span class="cm">/* librato (http://dev.librato.com/v1/post/metrics) source</span></td>
      </tr>
      <tr>
        <td id="L189" class="blob-num js-line-number" data-line-number="189"></td>
        <td id="LC189" class="blob-code js-file-line"><span class="cm"> * If you want to see an example of how to use this source, check: https://gist.github.com/drio/5792680</span></td>
      </tr>
      <tr>
        <td id="L190" class="blob-num js-line-number" data-line-number="190"></td>
        <td id="LC190" class="blob-code js-file-line"><span class="cm"> */</span></td>
      </tr>
      <tr>
        <td id="L191" class="blob-num js-line-number" data-line-number="191"></td>
        <td id="LC191" class="blob-code js-file-line"><span class="nx">cubism_contextPrototype</span><span class="p">.</span><span class="nx">librato</span> <span class="o">=</span> <span class="kd">function</span><span class="p">(</span><span class="nx">user</span><span class="p">,</span> <span class="nx">token</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L192" class="blob-num js-line-number" data-line-number="192"></td>
        <td id="LC192" class="blob-code js-file-line">  <span class="kd">var</span> <span class="nx">source</span>      <span class="o">=</span> <span class="p">{},</span></td>
      </tr>
      <tr>
        <td id="L193" class="blob-num js-line-number" data-line-number="193"></td>
        <td id="LC193" class="blob-code js-file-line">      <span class="nx">context</span>     <span class="o">=</span> <span class="k">this</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L194" class="blob-num js-line-number" data-line-number="194"></td>
        <td id="LC194" class="blob-code js-file-line">      <span class="nx">auth_string</span> <span class="o">=</span> <span class="s2">&quot;Basic &quot;</span> <span class="o">+</span> <span class="nx">btoa</span><span class="p">(</span><span class="nx">user</span> <span class="o">+</span> <span class="s2">&quot;:&quot;</span> <span class="o">+</span> <span class="nx">token</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L195" class="blob-num js-line-number" data-line-number="195"></td>
        <td id="LC195" class="blob-code js-file-line">      <span class="nx">avail_rsts</span>  <span class="o">=</span> <span class="p">[</span> <span class="mi">1</span><span class="p">,</span> <span class="mi">60</span><span class="p">,</span> <span class="mi">900</span><span class="p">,</span> <span class="mi">3600</span> <span class="p">];</span></td>
      </tr>
      <tr>
        <td id="L196" class="blob-num js-line-number" data-line-number="196"></td>
        <td id="LC196" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L197" class="blob-num js-line-number" data-line-number="197"></td>
        <td id="LC197" class="blob-code js-file-line">  <span class="cm">/* Given a step, find the best librato resolution to use.</span></td>
      </tr>
      <tr>
        <td id="L198" class="blob-num js-line-number" data-line-number="198"></td>
        <td id="LC198" class="blob-code js-file-line"><span class="cm">   *</span></td>
      </tr>
      <tr>
        <td id="L199" class="blob-num js-line-number" data-line-number="199"></td>
        <td id="LC199" class="blob-code js-file-line"><span class="cm">   * Example:</span></td>
      </tr>
      <tr>
        <td id="L200" class="blob-num js-line-number" data-line-number="200"></td>
        <td id="LC200" class="blob-code js-file-line"><span class="cm">   *</span></td>
      </tr>
      <tr>
        <td id="L201" class="blob-num js-line-number" data-line-number="201"></td>
        <td id="LC201" class="blob-code js-file-line"><span class="cm">   * (s) : cubism step</span></td>
      </tr>
      <tr>
        <td id="L202" class="blob-num js-line-number" data-line-number="202"></td>
        <td id="LC202" class="blob-code js-file-line"><span class="cm">   *</span></td>
      </tr>
      <tr>
        <td id="L203" class="blob-num js-line-number" data-line-number="203"></td>
        <td id="LC203" class="blob-code js-file-line"><span class="cm">   * avail_rsts   1 --------------- 60 --------------- 900 ---------------- 3600</span></td>
      </tr>
      <tr>
        <td id="L204" class="blob-num js-line-number" data-line-number="204"></td>
        <td id="LC204" class="blob-code js-file-line"><span class="cm">   *                                |    (s)            |</span></td>
      </tr>
      <tr>
        <td id="L205" class="blob-num js-line-number" data-line-number="205"></td>
        <td id="LC205" class="blob-code js-file-line"><span class="cm">   *                                |                   |</span></td>
      </tr>
      <tr>
        <td id="L206" class="blob-num js-line-number" data-line-number="206"></td>
        <td id="LC206" class="blob-code js-file-line"><span class="cm">   *                              [low_res             top_res]</span></td>
      </tr>
      <tr>
        <td id="L207" class="blob-num js-line-number" data-line-number="207"></td>
        <td id="LC207" class="blob-code js-file-line"><span class="cm">   *</span></td>
      </tr>
      <tr>
        <td id="L208" class="blob-num js-line-number" data-line-number="208"></td>
        <td id="LC208" class="blob-code js-file-line"><span class="cm">   * return: low_res (60)</span></td>
      </tr>
      <tr>
        <td id="L209" class="blob-num js-line-number" data-line-number="209"></td>
        <td id="LC209" class="blob-code js-file-line"><span class="cm">   */</span></td>
      </tr>
      <tr>
        <td id="L210" class="blob-num js-line-number" data-line-number="210"></td>
        <td id="LC210" class="blob-code js-file-line">  <span class="kd">function</span> <span class="nx">find_ideal_librato_resolution</span><span class="p">(</span><span class="nx">step</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L211" class="blob-num js-line-number" data-line-number="211"></td>
        <td id="LC211" class="blob-code js-file-line">    <span class="kd">var</span> <span class="nx">highest_res</span> <span class="o">=</span> <span class="nx">avail_rsts</span><span class="p">[</span><span class="mi">0</span><span class="p">],</span></td>
      </tr>
      <tr>
        <td id="L212" class="blob-num js-line-number" data-line-number="212"></td>
        <td id="LC212" class="blob-code js-file-line">        <span class="nx">lowest_res</span>  <span class="o">=</span> <span class="nx">avail_rsts</span><span class="p">[</span><span class="nx">avail_rsts</span><span class="p">.</span><span class="nx">length</span><span class="p">];</span> <span class="c1">// high and lowest available resolution from librato</span></td>
      </tr>
      <tr>
        <td id="L213" class="blob-num js-line-number" data-line-number="213"></td>
        <td id="LC213" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L214" class="blob-num js-line-number" data-line-number="214"></td>
        <td id="LC214" class="blob-code js-file-line">    <span class="cm">/* If step is outside the highest or lowest librato resolution, pick them and we are done */</span></td>
      </tr>
      <tr>
        <td id="L215" class="blob-num js-line-number" data-line-number="215"></td>
        <td id="LC215" class="blob-code js-file-line">    <span class="k">if</span> <span class="p">(</span><span class="nx">step</span> <span class="o">&gt;=</span> <span class="nx">lowest_res</span><span class="p">)</span></td>
      </tr>
      <tr>
        <td id="L216" class="blob-num js-line-number" data-line-number="216"></td>
        <td id="LC216" class="blob-code js-file-line">      <span class="k">return</span> <span class="nx">lowest_res</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L217" class="blob-num js-line-number" data-line-number="217"></td>
        <td id="LC217" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L218" class="blob-num js-line-number" data-line-number="218"></td>
        <td id="LC218" class="blob-code js-file-line">    <span class="k">if</span> <span class="p">(</span><span class="nx">step</span> <span class="o">&lt;=</span> <span class="nx">highest_res</span><span class="p">)</span></td>
      </tr>
      <tr>
        <td id="L219" class="blob-num js-line-number" data-line-number="219"></td>
        <td id="LC219" class="blob-code js-file-line">      <span class="k">return</span> <span class="nx">highest_res</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L220" class="blob-num js-line-number" data-line-number="220"></td>
        <td id="LC220" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L221" class="blob-num js-line-number" data-line-number="221"></td>
        <td id="LC221" class="blob-code js-file-line">    <span class="cm">/* If not, find in what resolution interval the step lands. */</span></td>
      </tr>
      <tr>
        <td id="L222" class="blob-num js-line-number" data-line-number="222"></td>
        <td id="LC222" class="blob-code js-file-line">    <span class="kd">var</span> <span class="nx">iof</span><span class="p">,</span> <span class="nx">top_res</span><span class="p">,</span> <span class="nx">i</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L223" class="blob-num js-line-number" data-line-number="223"></td>
        <td id="LC223" class="blob-code js-file-line">    <span class="k">for</span> <span class="p">(</span><span class="nx">i</span><span class="o">=</span><span class="nx">step</span><span class="p">;</span> <span class="nx">i</span><span class="o">&lt;=</span><span class="nx">lowest_res</span><span class="p">;</span> <span class="nx">i</span><span class="o">++</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L224" class="blob-num js-line-number" data-line-number="224"></td>
        <td id="LC224" class="blob-code js-file-line">      <span class="nx">iof</span> <span class="o">=</span> <span class="nx">avail_rsts</span><span class="p">.</span><span class="nx">indexOf</span><span class="p">(</span><span class="nx">i</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L225" class="blob-num js-line-number" data-line-number="225"></td>
        <td id="LC225" class="blob-code js-file-line">      <span class="k">if</span> <span class="p">(</span><span class="nx">iof</span> <span class="o">&gt;</span> <span class="o">-</span><span class="mi">1</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L226" class="blob-num js-line-number" data-line-number="226"></td>
        <td id="LC226" class="blob-code js-file-line">        <span class="nx">top_res</span> <span class="o">=</span> <span class="nx">avail_rsts</span><span class="p">[</span><span class="nx">iof</span><span class="p">];</span></td>
      </tr>
      <tr>
        <td id="L227" class="blob-num js-line-number" data-line-number="227"></td>
        <td id="LC227" class="blob-code js-file-line">        <span class="k">break</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L228" class="blob-num js-line-number" data-line-number="228"></td>
        <td id="LC228" class="blob-code js-file-line">      <span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L229" class="blob-num js-line-number" data-line-number="229"></td>
        <td id="LC229" class="blob-code js-file-line">    <span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L230" class="blob-num js-line-number" data-line-number="230"></td>
        <td id="LC230" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L231" class="blob-num js-line-number" data-line-number="231"></td>
        <td id="LC231" class="blob-code js-file-line">    <span class="kd">var</span> <span class="nx">low_res</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L232" class="blob-num js-line-number" data-line-number="232"></td>
        <td id="LC232" class="blob-code js-file-line">    <span class="k">for</span> <span class="p">(</span><span class="nx">i</span><span class="o">=</span><span class="nx">step</span><span class="p">;</span> <span class="nx">i</span><span class="o">&gt;=</span><span class="nx">highest_res</span><span class="p">;</span> <span class="nx">i</span><span class="o">--</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L233" class="blob-num js-line-number" data-line-number="233"></td>
        <td id="LC233" class="blob-code js-file-line">      <span class="nx">iof</span> <span class="o">=</span> <span class="nx">avail_rsts</span><span class="p">.</span><span class="nx">indexOf</span><span class="p">(</span><span class="nx">i</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L234" class="blob-num js-line-number" data-line-number="234"></td>
        <td id="LC234" class="blob-code js-file-line">      <span class="k">if</span> <span class="p">(</span><span class="nx">iof</span> <span class="o">&gt;</span> <span class="o">-</span><span class="mi">1</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L235" class="blob-num js-line-number" data-line-number="235"></td>
        <td id="LC235" class="blob-code js-file-line">        <span class="nx">low_res</span> <span class="o">=</span> <span class="nx">avail_rsts</span><span class="p">[</span><span class="nx">iof</span><span class="p">];</span></td>
      </tr>
      <tr>
        <td id="L236" class="blob-num js-line-number" data-line-number="236"></td>
        <td id="LC236" class="blob-code js-file-line">        <span class="k">break</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L237" class="blob-num js-line-number" data-line-number="237"></td>
        <td id="LC237" class="blob-code js-file-line">      <span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L238" class="blob-num js-line-number" data-line-number="238"></td>
        <td id="LC238" class="blob-code js-file-line">    <span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L239" class="blob-num js-line-number" data-line-number="239"></td>
        <td id="LC239" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L240" class="blob-num js-line-number" data-line-number="240"></td>
        <td id="LC240" class="blob-code js-file-line">    <span class="cm">/* What&#39;s the closest librato resolution given the step ? */</span></td>
      </tr>
      <tr>
        <td id="L241" class="blob-num js-line-number" data-line-number="241"></td>
        <td id="LC241" class="blob-code js-file-line">    <span class="k">return</span> <span class="p">((</span><span class="nx">top_res</span><span class="o">-</span><span class="nx">step</span><span class="p">)</span> <span class="o">&lt;</span> <span class="p">(</span><span class="nx">step</span><span class="o">-</span><span class="nx">low_res</span><span class="p">))</span> <span class="o">?</span> <span class="nx">top_res</span> <span class="o">:</span> <span class="nx">low_res</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L242" class="blob-num js-line-number" data-line-number="242"></td>
        <td id="LC242" class="blob-code js-file-line">  <span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L243" class="blob-num js-line-number" data-line-number="243"></td>
        <td id="LC243" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L244" class="blob-num js-line-number" data-line-number="244"></td>
        <td id="LC244" class="blob-code js-file-line">  <span class="kd">function</span> <span class="nx">find_librato_resolution</span><span class="p">(</span><span class="nx">sdate</span><span class="p">,</span> <span class="nx">edate</span><span class="p">,</span> <span class="nx">step</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L245" class="blob-num js-line-number" data-line-number="245"></td>
        <td id="LC245" class="blob-code js-file-line">    <span class="kd">var</span> <span class="nx">i_size</span>      <span class="o">=</span> <span class="nx">edate</span> <span class="o">-</span> <span class="nx">sdate</span><span class="p">,</span>                 <span class="c1">// interval size</span></td>
      </tr>
      <tr>
        <td id="L246" class="blob-num js-line-number" data-line-number="246"></td>
        <td id="LC246" class="blob-code js-file-line">        <span class="nx">month</span>       <span class="o">=</span> <span class="mi">2419200</span><span class="p">,</span></td>
      </tr>
      <tr>
        <td id="L247" class="blob-num js-line-number" data-line-number="247"></td>
        <td id="LC247" class="blob-code js-file-line">        <span class="nx">week</span>        <span class="o">=</span> <span class="mi">604800</span><span class="p">,</span></td>
      </tr>
      <tr>
        <td id="L248" class="blob-num js-line-number" data-line-number="248"></td>
        <td id="LC248" class="blob-code js-file-line">        <span class="nx">two_days</span>    <span class="o">=</span> <span class="mi">172800</span><span class="p">,</span></td>
      </tr>
      <tr>
        <td id="L249" class="blob-num js-line-number" data-line-number="249"></td>
        <td id="LC249" class="blob-code js-file-line">        <span class="nx">ideal_res</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L250" class="blob-num js-line-number" data-line-number="250"></td>
        <td id="LC250" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L251" class="blob-num js-line-number" data-line-number="251"></td>
        <td id="LC251" class="blob-code js-file-line">    <span class="k">if</span> <span class="p">(</span><span class="nx">i_size</span> <span class="o">&gt;</span> <span class="nx">month</span><span class="p">)</span></td>
      </tr>
      <tr>
        <td id="L252" class="blob-num js-line-number" data-line-number="252"></td>
        <td id="LC252" class="blob-code js-file-line">      <span class="k">return</span> <span class="mi">3600</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L253" class="blob-num js-line-number" data-line-number="253"></td>
        <td id="LC253" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L254" class="blob-num js-line-number" data-line-number="254"></td>
        <td id="LC254" class="blob-code js-file-line">    <span class="nx">ideal_res</span> <span class="o">=</span> <span class="nx">find_ideal_librato_resolution</span><span class="p">(</span><span class="nx">step</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L255" class="blob-num js-line-number" data-line-number="255"></td>
        <td id="LC255" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L256" class="blob-num js-line-number" data-line-number="256"></td>
        <td id="LC256" class="blob-code js-file-line">    <span class="cm">/*</span></td>
      </tr>
      <tr>
        <td id="L257" class="blob-num js-line-number" data-line-number="257"></td>
        <td id="LC257" class="blob-code js-file-line"><span class="cm">     * Now we have the ideal resolution, but due to the retention policies at librato, maybe we have</span></td>
      </tr>
      <tr>
        <td id="L258" class="blob-num js-line-number" data-line-number="258"></td>
        <td id="LC258" class="blob-code js-file-line"><span class="cm">     * to use a higher resolution.</span></td>
      </tr>
      <tr>
        <td id="L259" class="blob-num js-line-number" data-line-number="259"></td>
        <td id="LC259" class="blob-code js-file-line"><span class="cm">     * http://support.metrics.librato.com/knowledgebase/articles/66838-understanding-metrics-roll-ups-retention-and-grap</span></td>
      </tr>
      <tr>
        <td id="L260" class="blob-num js-line-number" data-line-number="260"></td>
        <td id="LC260" class="blob-code js-file-line"><span class="cm">     */</span></td>
      </tr>
      <tr>
        <td id="L261" class="blob-num js-line-number" data-line-number="261"></td>
        <td id="LC261" class="blob-code js-file-line">    <span class="k">if</span> <span class="p">(</span><span class="nx">i_size</span> <span class="o">&gt;</span> <span class="nx">week</span> <span class="o">&amp;&amp;</span> <span class="nx">ideal_res</span> <span class="o">&lt;</span> <span class="mi">900</span><span class="p">)</span></td>
      </tr>
      <tr>
        <td id="L262" class="blob-num js-line-number" data-line-number="262"></td>
        <td id="LC262" class="blob-code js-file-line">      <span class="k">return</span> <span class="mi">900</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L263" class="blob-num js-line-number" data-line-number="263"></td>
        <td id="LC263" class="blob-code js-file-line">    <span class="k">else</span> <span class="k">if</span> <span class="p">(</span><span class="nx">i_size</span> <span class="o">&gt;</span> <span class="nx">two_days</span> <span class="o">&amp;&amp;</span> <span class="nx">ideal_res</span> <span class="o">&lt;</span> <span class="mi">60</span><span class="p">)</span></td>
      </tr>
      <tr>
        <td id="L264" class="blob-num js-line-number" data-line-number="264"></td>
        <td id="LC264" class="blob-code js-file-line">      <span class="k">return</span> <span class="mi">60</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L265" class="blob-num js-line-number" data-line-number="265"></td>
        <td id="LC265" class="blob-code js-file-line">    <span class="k">else</span></td>
      </tr>
      <tr>
        <td id="L266" class="blob-num js-line-number" data-line-number="266"></td>
        <td id="LC266" class="blob-code js-file-line">      <span class="k">return</span> <span class="nx">ideal_res</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L267" class="blob-num js-line-number" data-line-number="267"></td>
        <td id="LC267" class="blob-code js-file-line">  <span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L268" class="blob-num js-line-number" data-line-number="268"></td>
        <td id="LC268" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L269" class="blob-num js-line-number" data-line-number="269"></td>
        <td id="LC269" class="blob-code js-file-line">  <span class="cm">/* All the logic to query the librato API is here */</span></td>
      </tr>
      <tr>
        <td id="L270" class="blob-num js-line-number" data-line-number="270"></td>
        <td id="LC270" class="blob-code js-file-line">  <span class="kd">var</span> <span class="nx">librato_request</span> <span class="o">=</span> <span class="kd">function</span><span class="p">(</span><span class="nx">composite</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L271" class="blob-num js-line-number" data-line-number="271"></td>
        <td id="LC271" class="blob-code js-file-line">    <span class="kd">var</span> <span class="nx">url_prefix</span>  <span class="o">=</span> <span class="s2">&quot;https://metrics-api.librato.com/v1/metrics&quot;</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L272" class="blob-num js-line-number" data-line-number="272"></td>
        <td id="LC272" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L273" class="blob-num js-line-number" data-line-number="273"></td>
        <td id="LC273" class="blob-code js-file-line">    <span class="kd">function</span> <span class="nx">make_url</span><span class="p">(</span><span class="nx">sdate</span><span class="p">,</span> <span class="nx">edate</span><span class="p">,</span> <span class="nx">step</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L274" class="blob-num js-line-number" data-line-number="274"></td>
        <td id="LC274" class="blob-code js-file-line">      <span class="kd">var</span> <span class="nx">params</span>    <span class="o">=</span> <span class="s2">&quot;compose=&quot;</span>     <span class="o">+</span> <span class="nx">composite</span> <span class="o">+</span></td>
      </tr>
      <tr>
        <td id="L275" class="blob-num js-line-number" data-line-number="275"></td>
        <td id="LC275" class="blob-code js-file-line">                      <span class="s2">&quot;&amp;start_time=&quot;</span> <span class="o">+</span> <span class="nx">sdate</span>     <span class="o">+</span></td>
      </tr>
      <tr>
        <td id="L276" class="blob-num js-line-number" data-line-number="276"></td>
        <td id="LC276" class="blob-code js-file-line">                      <span class="s2">&quot;&amp;end_time=&quot;</span>   <span class="o">+</span> <span class="nx">edate</span>     <span class="o">+</span></td>
      </tr>
      <tr>
        <td id="L277" class="blob-num js-line-number" data-line-number="277"></td>
        <td id="LC277" class="blob-code js-file-line">                      <span class="s2">&quot;&amp;resolution=&quot;</span> <span class="o">+</span> <span class="nx">find_librato_resolution</span><span class="p">(</span><span class="nx">sdate</span><span class="p">,</span> <span class="nx">edate</span><span class="p">,</span> <span class="nx">step</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L278" class="blob-num js-line-number" data-line-number="278"></td>
        <td id="LC278" class="blob-code js-file-line">      <span class="k">return</span> <span class="nx">url_prefix</span> <span class="o">+</span> <span class="s2">&quot;?&quot;</span> <span class="o">+</span> <span class="nx">params</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L279" class="blob-num js-line-number" data-line-number="279"></td>
        <td id="LC279" class="blob-code js-file-line">    <span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L280" class="blob-num js-line-number" data-line-number="280"></td>
        <td id="LC280" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L281" class="blob-num js-line-number" data-line-number="281"></td>
        <td id="LC281" class="blob-code js-file-line">    <span class="cm">/*</span></td>
      </tr>
      <tr>
        <td id="L282" class="blob-num js-line-number" data-line-number="282"></td>
        <td id="LC282" class="blob-code js-file-line"><span class="cm">     * We are most likely not going to get the same number of measurements</span></td>
      </tr>
      <tr>
        <td id="L283" class="blob-num js-line-number" data-line-number="283"></td>
        <td id="LC283" class="blob-code js-file-line"><span class="cm">     * cubism expects for a particular context: We have to perform down/up</span></td>
      </tr>
      <tr>
        <td id="L284" class="blob-num js-line-number" data-line-number="284"></td>
        <td id="LC284" class="blob-code js-file-line"><span class="cm">     * sampling</span></td>
      </tr>
      <tr>
        <td id="L285" class="blob-num js-line-number" data-line-number="285"></td>
        <td id="LC285" class="blob-code js-file-line"><span class="cm">     */</span></td>
      </tr>
      <tr>
        <td id="L286" class="blob-num js-line-number" data-line-number="286"></td>
        <td id="LC286" class="blob-code js-file-line">    <span class="kd">function</span> <span class="nx">down_up_sampling</span><span class="p">(</span><span class="nx">isdate</span><span class="p">,</span> <span class="nx">iedate</span><span class="p">,</span> <span class="nx">step</span><span class="p">,</span> <span class="nx">librato_mm</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L287" class="blob-num js-line-number" data-line-number="287"></td>
        <td id="LC287" class="blob-code js-file-line">      <span class="kd">var</span> <span class="nx">av</span> <span class="o">=</span> <span class="p">[];</span></td>
      </tr>
      <tr>
        <td id="L288" class="blob-num js-line-number" data-line-number="288"></td>
        <td id="LC288" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L289" class="blob-num js-line-number" data-line-number="289"></td>
        <td id="LC289" class="blob-code js-file-line">      <span class="k">for</span> <span class="p">(</span><span class="nx">i</span><span class="o">=</span><span class="nx">isdate</span><span class="p">;</span> <span class="nx">i</span><span class="o">&lt;=</span><span class="nx">iedate</span><span class="p">;</span> <span class="nx">i</span><span class="o">+=</span><span class="nx">step</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L290" class="blob-num js-line-number" data-line-number="290"></td>
        <td id="LC290" class="blob-code js-file-line">        <span class="kd">var</span> <span class="nx">int_mes</span> <span class="o">=</span> <span class="p">[];</span></td>
      </tr>
      <tr>
        <td id="L291" class="blob-num js-line-number" data-line-number="291"></td>
        <td id="LC291" class="blob-code js-file-line">        <span class="k">while</span> <span class="p">(</span><span class="nx">librato_mm</span><span class="p">.</span><span class="nx">length</span> <span class="o">&amp;&amp;</span> <span class="nx">librato_mm</span><span class="p">[</span><span class="mi">0</span><span class="p">].</span><span class="nx">measure_time</span> <span class="o">&lt;=</span> <span class="nx">i</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L292" class="blob-num js-line-number" data-line-number="292"></td>
        <td id="LC292" class="blob-code js-file-line">          <span class="nx">int_mes</span><span class="p">.</span><span class="nx">push</span><span class="p">(</span><span class="nx">librato_mm</span><span class="p">.</span><span class="nx">shift</span><span class="p">().</span><span class="nx">value</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L293" class="blob-num js-line-number" data-line-number="293"></td>
        <td id="LC293" class="blob-code js-file-line">        <span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L294" class="blob-num js-line-number" data-line-number="294"></td>
        <td id="LC294" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L295" class="blob-num js-line-number" data-line-number="295"></td>
        <td id="LC295" class="blob-code js-file-line">        <span class="kd">var</span> <span class="nx">v</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L296" class="blob-num js-line-number" data-line-number="296"></td>
        <td id="LC296" class="blob-code js-file-line">        <span class="k">if</span> <span class="p">(</span><span class="nx">int_mes</span><span class="p">.</span><span class="nx">length</span><span class="p">)</span> <span class="p">{</span> <span class="cm">/* Compute the average */</span></td>
      </tr>
      <tr>
        <td id="L297" class="blob-num js-line-number" data-line-number="297"></td>
        <td id="LC297" class="blob-code js-file-line">          <span class="nx">v</span> <span class="o">=</span> <span class="nx">int_mes</span><span class="p">.</span><span class="nx">reduce</span><span class="p">(</span><span class="kd">function</span><span class="p">(</span><span class="nx">a</span><span class="p">,</span> <span class="nx">b</span><span class="p">)</span> <span class="p">{</span> <span class="k">return</span> <span class="nx">a</span> <span class="o">+</span> <span class="nx">b</span> <span class="p">})</span> <span class="o">/</span> <span class="nx">int_mes</span><span class="p">.</span><span class="nx">length</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L298" class="blob-num js-line-number" data-line-number="298"></td>
        <td id="LC298" class="blob-code js-file-line">        <span class="p">}</span> <span class="k">else</span> <span class="p">{</span> <span class="cm">/* No librato values on interval */</span></td>
      </tr>
      <tr>
        <td id="L299" class="blob-num js-line-number" data-line-number="299"></td>
        <td id="LC299" class="blob-code js-file-line">          <span class="nx">v</span> <span class="o">=</span> <span class="p">(</span><span class="nx">av</span><span class="p">.</span><span class="nx">length</span><span class="p">)</span> <span class="o">?</span> <span class="nx">av</span><span class="p">[</span><span class="nx">av</span><span class="p">.</span><span class="nx">length</span><span class="o">-</span><span class="mi">1</span><span class="p">]</span> <span class="o">:</span> <span class="mi">0</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L300" class="blob-num js-line-number" data-line-number="300"></td>
        <td id="LC300" class="blob-code js-file-line">        <span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L301" class="blob-num js-line-number" data-line-number="301"></td>
        <td id="LC301" class="blob-code js-file-line">        <span class="nx">av</span><span class="p">.</span><span class="nx">push</span><span class="p">(</span><span class="nx">v</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L302" class="blob-num js-line-number" data-line-number="302"></td>
        <td id="LC302" class="blob-code js-file-line">      <span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L303" class="blob-num js-line-number" data-line-number="303"></td>
        <td id="LC303" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L304" class="blob-num js-line-number" data-line-number="304"></td>
        <td id="LC304" class="blob-code js-file-line">      <span class="k">return</span> <span class="nx">av</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L305" class="blob-num js-line-number" data-line-number="305"></td>
        <td id="LC305" class="blob-code js-file-line">    <span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L306" class="blob-num js-line-number" data-line-number="306"></td>
        <td id="LC306" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L307" class="blob-num js-line-number" data-line-number="307"></td>
        <td id="LC307" class="blob-code js-file-line">    <span class="nx">request</span> <span class="o">=</span> <span class="p">{};</span></td>
      </tr>
      <tr>
        <td id="L308" class="blob-num js-line-number" data-line-number="308"></td>
        <td id="LC308" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L309" class="blob-num js-line-number" data-line-number="309"></td>
        <td id="LC309" class="blob-code js-file-line">    <span class="nx">request</span><span class="p">.</span><span class="nx">fire</span> <span class="o">=</span> <span class="kd">function</span><span class="p">(</span><span class="nx">isdate</span><span class="p">,</span> <span class="nx">iedate</span><span class="p">,</span> <span class="nx">step</span><span class="p">,</span> <span class="nx">callback_done</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L310" class="blob-num js-line-number" data-line-number="310"></td>
        <td id="LC310" class="blob-code js-file-line">      <span class="kd">var</span> <span class="nx">a_values</span> <span class="o">=</span> <span class="p">[];</span> <span class="cm">/* Store partial values from librato */</span></td>
      </tr>
      <tr>
        <td id="L311" class="blob-num js-line-number" data-line-number="311"></td>
        <td id="LC311" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L312" class="blob-num js-line-number" data-line-number="312"></td>
        <td id="LC312" class="blob-code js-file-line">      <span class="cm">/*</span></td>
      </tr>
      <tr>
        <td id="L313" class="blob-num js-line-number" data-line-number="313"></td>
        <td id="LC313" class="blob-code js-file-line"><span class="cm">       * Librato has a limit in the number of measurements we get back in a request (100).</span></td>
      </tr>
      <tr>
        <td id="L314" class="blob-num js-line-number" data-line-number="314"></td>
        <td id="LC314" class="blob-code js-file-line"><span class="cm">       * We recursively perform requests to the API to ensure we have all the data points</span></td>
      </tr>
      <tr>
        <td id="L315" class="blob-num js-line-number" data-line-number="315"></td>
        <td id="LC315" class="blob-code js-file-line"><span class="cm">       * for the interval we are working on.</span></td>
      </tr>
      <tr>
        <td id="L316" class="blob-num js-line-number" data-line-number="316"></td>
        <td id="LC316" class="blob-code js-file-line"><span class="cm">       */</span></td>
      </tr>
      <tr>
        <td id="L317" class="blob-num js-line-number" data-line-number="317"></td>
        <td id="LC317" class="blob-code js-file-line">      <span class="kd">function</span> <span class="nx">actual_request</span><span class="p">(</span><span class="nx">full_url</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L318" class="blob-num js-line-number" data-line-number="318"></td>
        <td id="LC318" class="blob-code js-file-line">        <span class="nx">d3</span><span class="p">.</span><span class="nx">json</span><span class="p">(</span><span class="nx">full_url</span><span class="p">)</span></td>
      </tr>
      <tr>
        <td id="L319" class="blob-num js-line-number" data-line-number="319"></td>
        <td id="LC319" class="blob-code js-file-line">          <span class="p">.</span><span class="nx">header</span><span class="p">(</span><span class="s2">&quot;X-Requested-With&quot;</span><span class="p">,</span> <span class="s2">&quot;XMLHttpRequest&quot;</span><span class="p">)</span></td>
      </tr>
      <tr>
        <td id="L320" class="blob-num js-line-number" data-line-number="320"></td>
        <td id="LC320" class="blob-code js-file-line">          <span class="p">.</span><span class="nx">header</span><span class="p">(</span><span class="s2">&quot;Authorization&quot;</span><span class="p">,</span> <span class="nx">auth_string</span><span class="p">)</span></td>
      </tr>
      <tr>
        <td id="L321" class="blob-num js-line-number" data-line-number="321"></td>
        <td id="LC321" class="blob-code js-file-line">          <span class="p">.</span><span class="nx">header</span><span class="p">(</span><span class="s2">&quot;Librato-User-Agent&quot;</span><span class="p">,</span> <span class="s1">&#39;cubism/&#39;</span> <span class="o">+</span> <span class="nx">cubism</span><span class="p">.</span><span class="nx">version</span><span class="p">)</span></td>
      </tr>
      <tr>
        <td id="L322" class="blob-num js-line-number" data-line-number="322"></td>
        <td id="LC322" class="blob-code js-file-line">          <span class="p">.</span><span class="nx">get</span><span class="p">(</span><span class="kd">function</span> <span class="p">(</span><span class="nx">error</span><span class="p">,</span> <span class="nx">data</span><span class="p">)</span> <span class="p">{</span> <span class="cm">/* Callback; data available */</span></td>
      </tr>
      <tr>
        <td id="L323" class="blob-num js-line-number" data-line-number="323"></td>
        <td id="LC323" class="blob-code js-file-line">            <span class="k">if</span> <span class="p">(</span><span class="o">!</span><span class="nx">error</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L324" class="blob-num js-line-number" data-line-number="324"></td>
        <td id="LC324" class="blob-code js-file-line">              <span class="k">if</span> <span class="p">(</span><span class="nx">data</span><span class="p">.</span><span class="nx">measurements</span><span class="p">.</span><span class="nx">length</span> <span class="o">===</span> <span class="mi">0</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L325" class="blob-num js-line-number" data-line-number="325"></td>
        <td id="LC325" class="blob-code js-file-line">                <span class="k">return</span></td>
      </tr>
      <tr>
        <td id="L326" class="blob-num js-line-number" data-line-number="326"></td>
        <td id="LC326" class="blob-code js-file-line">              <span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L327" class="blob-num js-line-number" data-line-number="327"></td>
        <td id="LC327" class="blob-code js-file-line">              <span class="nx">data</span><span class="p">.</span><span class="nx">measurements</span><span class="p">[</span><span class="mi">0</span><span class="p">].</span><span class="nx">series</span><span class="p">.</span><span class="nx">forEach</span><span class="p">(</span><span class="kd">function</span><span class="p">(</span><span class="nx">o</span><span class="p">)</span> <span class="p">{</span> <span class="nx">a_values</span><span class="p">.</span><span class="nx">push</span><span class="p">(</span><span class="nx">o</span><span class="p">);</span> <span class="p">});</span></td>
      </tr>
      <tr>
        <td id="L328" class="blob-num js-line-number" data-line-number="328"></td>
        <td id="LC328" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L329" class="blob-num js-line-number" data-line-number="329"></td>
        <td id="LC329" class="blob-code js-file-line">              <span class="kd">var</span> <span class="nx">still_more_values</span> <span class="o">=</span> <span class="s1">&#39;query&#39;</span> <span class="k">in</span> <span class="nx">data</span> <span class="o">&amp;&amp;</span> <span class="s1">&#39;next_time&#39;</span> <span class="k">in</span> <span class="nx">data</span><span class="p">.</span><span class="nx">query</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L330" class="blob-num js-line-number" data-line-number="330"></td>
        <td id="LC330" class="blob-code js-file-line">              <span class="k">if</span> <span class="p">(</span><span class="nx">still_more_values</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L331" class="blob-num js-line-number" data-line-number="331"></td>
        <td id="LC331" class="blob-code js-file-line">                <span class="nx">actual_request</span><span class="p">(</span><span class="nx">make_url</span><span class="p">(</span><span class="nx">data</span><span class="p">.</span><span class="nx">query</span><span class="p">.</span><span class="nx">next_time</span><span class="p">,</span> <span class="nx">iedate</span><span class="p">,</span> <span class="nx">step</span><span class="p">));</span></td>
      </tr>
      <tr>
        <td id="L332" class="blob-num js-line-number" data-line-number="332"></td>
        <td id="LC332" class="blob-code js-file-line">              <span class="p">}</span> <span class="k">else</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L333" class="blob-num js-line-number" data-line-number="333"></td>
        <td id="LC333" class="blob-code js-file-line">                <span class="kd">var</span> <span class="nx">a_adjusted</span> <span class="o">=</span> <span class="nx">down_up_sampling</span><span class="p">(</span><span class="nx">isdate</span><span class="p">,</span> <span class="nx">iedate</span><span class="p">,</span> <span class="nx">step</span><span class="p">,</span> <span class="nx">a_values</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L334" class="blob-num js-line-number" data-line-number="334"></td>
        <td id="LC334" class="blob-code js-file-line">                <span class="nx">callback_done</span><span class="p">(</span><span class="nx">a_adjusted</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L335" class="blob-num js-line-number" data-line-number="335"></td>
        <td id="LC335" class="blob-code js-file-line">              <span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L336" class="blob-num js-line-number" data-line-number="336"></td>
        <td id="LC336" class="blob-code js-file-line">            <span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L337" class="blob-num js-line-number" data-line-number="337"></td>
        <td id="LC337" class="blob-code js-file-line">          <span class="p">});</span></td>
      </tr>
      <tr>
        <td id="L338" class="blob-num js-line-number" data-line-number="338"></td>
        <td id="LC338" class="blob-code js-file-line">      <span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L339" class="blob-num js-line-number" data-line-number="339"></td>
        <td id="LC339" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L340" class="blob-num js-line-number" data-line-number="340"></td>
        <td id="LC340" class="blob-code js-file-line">      <span class="nx">actual_request</span><span class="p">(</span><span class="nx">make_url</span><span class="p">(</span><span class="nx">isdate</span><span class="p">,</span> <span class="nx">iedate</span><span class="p">,</span> <span class="nx">step</span><span class="p">));</span></td>
      </tr>
      <tr>
        <td id="L341" class="blob-num js-line-number" data-line-number="341"></td>
        <td id="LC341" class="blob-code js-file-line">    <span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L342" class="blob-num js-line-number" data-line-number="342"></td>
        <td id="LC342" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L343" class="blob-num js-line-number" data-line-number="343"></td>
        <td id="LC343" class="blob-code js-file-line">    <span class="k">return</span> <span class="nx">request</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L344" class="blob-num js-line-number" data-line-number="344"></td>
        <td id="LC344" class="blob-code js-file-line">  <span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L345" class="blob-num js-line-number" data-line-number="345"></td>
        <td id="LC345" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L346" class="blob-num js-line-number" data-line-number="346"></td>
        <td id="LC346" class="blob-code js-file-line">  <span class="cm">/*</span></td>
      </tr>
      <tr>
        <td id="L347" class="blob-num js-line-number" data-line-number="347"></td>
        <td id="LC347" class="blob-code js-file-line"><span class="cm">   * The user will use this method to create a cubism source (librato in this case)</span></td>
      </tr>
      <tr>
        <td id="L348" class="blob-num js-line-number" data-line-number="348"></td>
        <td id="LC348" class="blob-code js-file-line"><span class="cm">   * and call .metric() as necessary to create metrics.</span></td>
      </tr>
      <tr>
        <td id="L349" class="blob-num js-line-number" data-line-number="349"></td>
        <td id="LC349" class="blob-code js-file-line"><span class="cm">   */</span></td>
      </tr>
      <tr>
        <td id="L350" class="blob-num js-line-number" data-line-number="350"></td>
        <td id="LC350" class="blob-code js-file-line">  <span class="nx">source</span><span class="p">.</span><span class="nx">metric</span> <span class="o">=</span> <span class="kd">function</span><span class="p">(</span><span class="nx">m_composite</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L351" class="blob-num js-line-number" data-line-number="351"></td>
        <td id="LC351" class="blob-code js-file-line">    <span class="k">return</span> <span class="nx">context</span><span class="p">.</span><span class="nx">metric</span><span class="p">(</span><span class="kd">function</span><span class="p">(</span><span class="nx">start</span><span class="p">,</span> <span class="nx">stop</span><span class="p">,</span> <span class="nx">step</span><span class="p">,</span> <span class="nx">callback</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L352" class="blob-num js-line-number" data-line-number="352"></td>
        <td id="LC352" class="blob-code js-file-line">      <span class="cm">/* All the librato logic is here; .fire() retrieves the metrics&#39; data */</span></td>
      </tr>
      <tr>
        <td id="L353" class="blob-num js-line-number" data-line-number="353"></td>
        <td id="LC353" class="blob-code js-file-line">      <span class="nx">librato_request</span><span class="p">(</span><span class="nx">m_composite</span><span class="p">)</span></td>
      </tr>
      <tr>
        <td id="L354" class="blob-num js-line-number" data-line-number="354"></td>
        <td id="LC354" class="blob-code js-file-line">        <span class="p">.</span><span class="nx">fire</span><span class="p">(</span><span class="nx">cubism_libratoFormatDate</span><span class="p">(</span><span class="nx">start</span><span class="p">),</span></td>
      </tr>
      <tr>
        <td id="L355" class="blob-num js-line-number" data-line-number="355"></td>
        <td id="LC355" class="blob-code js-file-line">              <span class="nx">cubism_libratoFormatDate</span><span class="p">(</span><span class="nx">stop</span><span class="p">),</span></td>
      </tr>
      <tr>
        <td id="L356" class="blob-num js-line-number" data-line-number="356"></td>
        <td id="LC356" class="blob-code js-file-line">              <span class="nx">cubism_libratoFormatDate</span><span class="p">(</span><span class="nx">step</span><span class="p">),</span></td>
      </tr>
      <tr>
        <td id="L357" class="blob-num js-line-number" data-line-number="357"></td>
        <td id="LC357" class="blob-code js-file-line">              <span class="kd">function</span><span class="p">(</span><span class="nx">a_values</span><span class="p">)</span> <span class="p">{</span> <span class="nx">callback</span><span class="p">(</span><span class="kc">null</span><span class="p">,</span> <span class="nx">a_values</span><span class="p">);</span> <span class="p">});</span></td>
      </tr>
      <tr>
        <td id="L358" class="blob-num js-line-number" data-line-number="358"></td>
        <td id="LC358" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L359" class="blob-num js-line-number" data-line-number="359"></td>
        <td id="LC359" class="blob-code js-file-line">      <span class="p">},</span> <span class="nx">m_composite</span> <span class="o">+=</span> <span class="s2">&quot;&quot;</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L360" class="blob-num js-line-number" data-line-number="360"></td>
        <td id="LC360" class="blob-code js-file-line">    <span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L361" class="blob-num js-line-number" data-line-number="361"></td>
        <td id="LC361" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L362" class="blob-num js-line-number" data-line-number="362"></td>
        <td id="LC362" class="blob-code js-file-line">  <span class="cm">/* This is not used when the source is librato */</span></td>
      </tr>
      <tr>
        <td id="L363" class="blob-num js-line-number" data-line-number="363"></td>
        <td id="LC363" class="blob-code js-file-line">  <span class="nx">source</span><span class="p">.</span><span class="nx">toString</span> <span class="o">=</span> <span class="kd">function</span><span class="p">()</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L364" class="blob-num js-line-number" data-line-number="364"></td>
        <td id="LC364" class="blob-code js-file-line">    <span class="k">return</span> <span class="s2">&quot;librato&quot;</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L365" class="blob-num js-line-number" data-line-number="365"></td>
        <td id="LC365" class="blob-code js-file-line">  <span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L366" class="blob-num js-line-number" data-line-number="366"></td>
        <td id="LC366" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L367" class="blob-num js-line-number" data-line-number="367"></td>
        <td id="LC367" class="blob-code js-file-line">  <span class="k">return</span> <span class="nx">source</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L368" class="blob-num js-line-number" data-line-number="368"></td>
        <td id="LC368" class="blob-code js-file-line"><span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L369" class="blob-num js-line-number" data-line-number="369"></td>
        <td id="LC369" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L370" class="blob-num js-line-number" data-line-number="370"></td>
        <td id="LC370" class="blob-code js-file-line"><span class="kd">var</span> <span class="nx">cubism_libratoFormatDate</span> <span class="o">=</span> <span class="kd">function</span><span class="p">(</span><span class="nx">time</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L371" class="blob-num js-line-number" data-line-number="371"></td>
        <td id="LC371" class="blob-code js-file-line">  <span class="k">return</span> <span class="nb">Math</span><span class="p">.</span><span class="nx">floor</span><span class="p">(</span><span class="nx">time</span> <span class="o">/</span> <span class="mi">1000</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L372" class="blob-num js-line-number" data-line-number="372"></td>
        <td id="LC372" class="blob-code js-file-line"><span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L373" class="blob-num js-line-number" data-line-number="373"></td>
        <td id="LC373" class="blob-code js-file-line"><span class="nx">cubism_contextPrototype</span><span class="p">.</span><span class="nx">graphite</span> <span class="o">=</span> <span class="kd">function</span><span class="p">(</span><span class="nx">host</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L374" class="blob-num js-line-number" data-line-number="374"></td>
        <td id="LC374" class="blob-code js-file-line">  <span class="k">if</span> <span class="p">(</span><span class="o">!</span><span class="nx">arguments</span><span class="p">.</span><span class="nx">length</span><span class="p">)</span> <span class="nx">host</span> <span class="o">=</span> <span class="s2">&quot;&quot;</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L375" class="blob-num js-line-number" data-line-number="375"></td>
        <td id="LC375" class="blob-code js-file-line">  <span class="kd">var</span> <span class="nx">source</span> <span class="o">=</span> <span class="p">{},</span></td>
      </tr>
      <tr>
        <td id="L376" class="blob-num js-line-number" data-line-number="376"></td>
        <td id="LC376" class="blob-code js-file-line">      <span class="nx">context</span> <span class="o">=</span> <span class="k">this</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L377" class="blob-num js-line-number" data-line-number="377"></td>
        <td id="LC377" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L378" class="blob-num js-line-number" data-line-number="378"></td>
        <td id="LC378" class="blob-code js-file-line">  <span class="nx">source</span><span class="p">.</span><span class="nx">metric</span> <span class="o">=</span> <span class="kd">function</span><span class="p">(</span><span class="nx">expression</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L379" class="blob-num js-line-number" data-line-number="379"></td>
        <td id="LC379" class="blob-code js-file-line">    <span class="kd">var</span> <span class="nx">sum</span> <span class="o">=</span> <span class="s2">&quot;sum&quot;</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L380" class="blob-num js-line-number" data-line-number="380"></td>
        <td id="LC380" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L381" class="blob-num js-line-number" data-line-number="381"></td>
        <td id="LC381" class="blob-code js-file-line">    <span class="kd">var</span> <span class="nx">metric</span> <span class="o">=</span> <span class="nx">context</span><span class="p">.</span><span class="nx">metric</span><span class="p">(</span><span class="kd">function</span><span class="p">(</span><span class="nx">start</span><span class="p">,</span> <span class="nx">stop</span><span class="p">,</span> <span class="nx">step</span><span class="p">,</span> <span class="nx">callback</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L382" class="blob-num js-line-number" data-line-number="382"></td>
        <td id="LC382" class="blob-code js-file-line">      <span class="kd">var</span> <span class="nx">target</span> <span class="o">=</span> <span class="nx">expression</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L383" class="blob-num js-line-number" data-line-number="383"></td>
        <td id="LC383" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L384" class="blob-num js-line-number" data-line-number="384"></td>
        <td id="LC384" class="blob-code js-file-line">      <span class="c1">// Apply the summarize, if necessary.</span></td>
      </tr>
      <tr>
        <td id="L385" class="blob-num js-line-number" data-line-number="385"></td>
        <td id="LC385" class="blob-code js-file-line">      <span class="k">if</span> <span class="p">(</span><span class="nx">step</span> <span class="o">!==</span> <span class="mi">1</span><span class="nx">e4</span><span class="p">)</span> <span class="nx">target</span> <span class="o">=</span> <span class="s2">&quot;summarize(&quot;</span> <span class="o">+</span> <span class="nx">target</span> <span class="o">+</span> <span class="s2">&quot;,&#39;&quot;</span></td>
      </tr>
      <tr>
        <td id="L386" class="blob-num js-line-number" data-line-number="386"></td>
        <td id="LC386" class="blob-code js-file-line">          <span class="o">+</span> <span class="p">(</span><span class="o">!</span><span class="p">(</span><span class="nx">step</span> <span class="o">%</span> <span class="mi">36</span><span class="nx">e5</span><span class="p">)</span> <span class="o">?</span> <span class="nx">step</span> <span class="o">/</span> <span class="mi">36</span><span class="nx">e5</span> <span class="o">+</span> <span class="s2">&quot;hour&quot;</span> <span class="o">:</span> <span class="o">!</span><span class="p">(</span><span class="nx">step</span> <span class="o">%</span> <span class="mi">6</span><span class="nx">e4</span><span class="p">)</span> <span class="o">?</span> <span class="nx">step</span> <span class="o">/</span> <span class="mi">6</span><span class="nx">e4</span> <span class="o">+</span> <span class="s2">&quot;min&quot;</span> <span class="o">:</span> <span class="nx">step</span> <span class="o">/</span> <span class="mi">1</span><span class="nx">e3</span> <span class="o">+</span> <span class="s2">&quot;sec&quot;</span><span class="p">)</span></td>
      </tr>
      <tr>
        <td id="L387" class="blob-num js-line-number" data-line-number="387"></td>
        <td id="LC387" class="blob-code js-file-line">          <span class="o">+</span> <span class="s2">&quot;&#39;,&#39;&quot;</span> <span class="o">+</span> <span class="nx">sum</span> <span class="o">+</span> <span class="s2">&quot;&#39;)&quot;</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L388" class="blob-num js-line-number" data-line-number="388"></td>
        <td id="LC388" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L389" class="blob-num js-line-number" data-line-number="389"></td>
        <td id="LC389" class="blob-code js-file-line">      <span class="nx">d3</span><span class="p">.</span><span class="nx">text</span><span class="p">(</span><span class="nx">host</span> <span class="o">+</span> <span class="s2">&quot;/render?format=raw&quot;</span></td>
      </tr>
      <tr>
        <td id="L390" class="blob-num js-line-number" data-line-number="390"></td>
        <td id="LC390" class="blob-code js-file-line">          <span class="o">+</span> <span class="s2">&quot;&amp;target=&quot;</span> <span class="o">+</span> <span class="nb">encodeURIComponent</span><span class="p">(</span><span class="s2">&quot;alias(&quot;</span> <span class="o">+</span> <span class="nx">target</span> <span class="o">+</span> <span class="s2">&quot;,&#39;&#39;)&quot;</span><span class="p">)</span></td>
      </tr>
      <tr>
        <td id="L391" class="blob-num js-line-number" data-line-number="391"></td>
        <td id="LC391" class="blob-code js-file-line">          <span class="o">+</span> <span class="s2">&quot;&amp;from=&quot;</span> <span class="o">+</span> <span class="nx">cubism_graphiteFormatDate</span><span class="p">(</span><span class="nx">start</span> <span class="o">-</span> <span class="mi">2</span> <span class="o">*</span> <span class="nx">step</span><span class="p">)</span> <span class="c1">// off-by-two?</span></td>
      </tr>
      <tr>
        <td id="L392" class="blob-num js-line-number" data-line-number="392"></td>
        <td id="LC392" class="blob-code js-file-line">          <span class="o">+</span> <span class="s2">&quot;&amp;until=&quot;</span> <span class="o">+</span> <span class="nx">cubism_graphiteFormatDate</span><span class="p">(</span><span class="nx">stop</span> <span class="o">-</span> <span class="mi">1000</span><span class="p">),</span> <span class="kd">function</span><span class="p">(</span><span class="nx">text</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L393" class="blob-num js-line-number" data-line-number="393"></td>
        <td id="LC393" class="blob-code js-file-line">        <span class="k">if</span> <span class="p">(</span><span class="o">!</span><span class="nx">text</span><span class="p">)</span> <span class="k">return</span> <span class="nx">callback</span><span class="p">(</span><span class="k">new</span> <span class="nb">Error</span><span class="p">(</span><span class="s2">&quot;unable to load data&quot;</span><span class="p">));</span></td>
      </tr>
      <tr>
        <td id="L394" class="blob-num js-line-number" data-line-number="394"></td>
        <td id="LC394" class="blob-code js-file-line">        <span class="nx">callback</span><span class="p">(</span><span class="kc">null</span><span class="p">,</span> <span class="nx">cubism_graphiteParse</span><span class="p">(</span><span class="nx">text</span><span class="p">));</span></td>
      </tr>
      <tr>
        <td id="L395" class="blob-num js-line-number" data-line-number="395"></td>
        <td id="LC395" class="blob-code js-file-line">      <span class="p">});</span></td>
      </tr>
      <tr>
        <td id="L396" class="blob-num js-line-number" data-line-number="396"></td>
        <td id="LC396" class="blob-code js-file-line">    <span class="p">},</span> <span class="nx">expression</span> <span class="o">+=</span> <span class="s2">&quot;&quot;</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L397" class="blob-num js-line-number" data-line-number="397"></td>
        <td id="LC397" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L398" class="blob-num js-line-number" data-line-number="398"></td>
        <td id="LC398" class="blob-code js-file-line">    <span class="nx">metric</span><span class="p">.</span><span class="nx">summarize</span> <span class="o">=</span> <span class="kd">function</span><span class="p">(</span><span class="nx">_</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L399" class="blob-num js-line-number" data-line-number="399"></td>
        <td id="LC399" class="blob-code js-file-line">      <span class="nx">sum</span> <span class="o">=</span> <span class="nx">_</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L400" class="blob-num js-line-number" data-line-number="400"></td>
        <td id="LC400" class="blob-code js-file-line">      <span class="k">return</span> <span class="nx">metric</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L401" class="blob-num js-line-number" data-line-number="401"></td>
        <td id="LC401" class="blob-code js-file-line">    <span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L402" class="blob-num js-line-number" data-line-number="402"></td>
        <td id="LC402" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L403" class="blob-num js-line-number" data-line-number="403"></td>
        <td id="LC403" class="blob-code js-file-line">    <span class="k">return</span> <span class="nx">metric</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L404" class="blob-num js-line-number" data-line-number="404"></td>
        <td id="LC404" class="blob-code js-file-line">  <span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L405" class="blob-num js-line-number" data-line-number="405"></td>
        <td id="LC405" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L406" class="blob-num js-line-number" data-line-number="406"></td>
        <td id="LC406" class="blob-code js-file-line">  <span class="nx">source</span><span class="p">.</span><span class="nx">find</span> <span class="o">=</span> <span class="kd">function</span><span class="p">(</span><span class="nx">pattern</span><span class="p">,</span> <span class="nx">callback</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L407" class="blob-num js-line-number" data-line-number="407"></td>
        <td id="LC407" class="blob-code js-file-line">    <span class="nx">d3</span><span class="p">.</span><span class="nx">json</span><span class="p">(</span><span class="nx">host</span> <span class="o">+</span> <span class="s2">&quot;/metrics/find?format=completer&quot;</span></td>
      </tr>
      <tr>
        <td id="L408" class="blob-num js-line-number" data-line-number="408"></td>
        <td id="LC408" class="blob-code js-file-line">        <span class="o">+</span> <span class="s2">&quot;&amp;query=&quot;</span> <span class="o">+</span> <span class="nb">encodeURIComponent</span><span class="p">(</span><span class="nx">pattern</span><span class="p">),</span> <span class="kd">function</span><span class="p">(</span><span class="nx">result</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L409" class="blob-num js-line-number" data-line-number="409"></td>
        <td id="LC409" class="blob-code js-file-line">      <span class="k">if</span> <span class="p">(</span><span class="o">!</span><span class="nx">result</span><span class="p">)</span> <span class="k">return</span> <span class="nx">callback</span><span class="p">(</span><span class="k">new</span> <span class="nb">Error</span><span class="p">(</span><span class="s2">&quot;unable to find metrics&quot;</span><span class="p">));</span></td>
      </tr>
      <tr>
        <td id="L410" class="blob-num js-line-number" data-line-number="410"></td>
        <td id="LC410" class="blob-code js-file-line">      <span class="nx">callback</span><span class="p">(</span><span class="kc">null</span><span class="p">,</span> <span class="nx">result</span><span class="p">.</span><span class="nx">metrics</span><span class="p">.</span><span class="nx">map</span><span class="p">(</span><span class="kd">function</span><span class="p">(</span><span class="nx">d</span><span class="p">)</span> <span class="p">{</span> <span class="k">return</span> <span class="nx">d</span><span class="p">.</span><span class="nx">path</span><span class="p">;</span> <span class="p">}));</span></td>
      </tr>
      <tr>
        <td id="L411" class="blob-num js-line-number" data-line-number="411"></td>
        <td id="LC411" class="blob-code js-file-line">    <span class="p">});</span></td>
      </tr>
      <tr>
        <td id="L412" class="blob-num js-line-number" data-line-number="412"></td>
        <td id="LC412" class="blob-code js-file-line">  <span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L413" class="blob-num js-line-number" data-line-number="413"></td>
        <td id="LC413" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L414" class="blob-num js-line-number" data-line-number="414"></td>
        <td id="LC414" class="blob-code js-file-line">  <span class="c1">// Returns the graphite host.</span></td>
      </tr>
      <tr>
        <td id="L415" class="blob-num js-line-number" data-line-number="415"></td>
        <td id="LC415" class="blob-code js-file-line">  <span class="nx">source</span><span class="p">.</span><span class="nx">toString</span> <span class="o">=</span> <span class="kd">function</span><span class="p">()</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L416" class="blob-num js-line-number" data-line-number="416"></td>
        <td id="LC416" class="blob-code js-file-line">    <span class="k">return</span> <span class="nx">host</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L417" class="blob-num js-line-number" data-line-number="417"></td>
        <td id="LC417" class="blob-code js-file-line">  <span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L418" class="blob-num js-line-number" data-line-number="418"></td>
        <td id="LC418" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L419" class="blob-num js-line-number" data-line-number="419"></td>
        <td id="LC419" class="blob-code js-file-line">  <span class="k">return</span> <span class="nx">source</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L420" class="blob-num js-line-number" data-line-number="420"></td>
        <td id="LC420" class="blob-code js-file-line"><span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L421" class="blob-num js-line-number" data-line-number="421"></td>
        <td id="LC421" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L422" class="blob-num js-line-number" data-line-number="422"></td>
        <td id="LC422" class="blob-code js-file-line"><span class="c1">// Graphite understands seconds since UNIX epoch.</span></td>
      </tr>
      <tr>
        <td id="L423" class="blob-num js-line-number" data-line-number="423"></td>
        <td id="LC423" class="blob-code js-file-line"><span class="kd">function</span> <span class="nx">cubism_graphiteFormatDate</span><span class="p">(</span><span class="nx">time</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L424" class="blob-num js-line-number" data-line-number="424"></td>
        <td id="LC424" class="blob-code js-file-line">  <span class="k">return</span> <span class="nb">Math</span><span class="p">.</span><span class="nx">floor</span><span class="p">(</span><span class="nx">time</span> <span class="o">/</span> <span class="mi">1000</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L425" class="blob-num js-line-number" data-line-number="425"></td>
        <td id="LC425" class="blob-code js-file-line"><span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L426" class="blob-num js-line-number" data-line-number="426"></td>
        <td id="LC426" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L427" class="blob-num js-line-number" data-line-number="427"></td>
        <td id="LC427" class="blob-code js-file-line"><span class="c1">// Helper method for parsing graphite&#39;s raw format.</span></td>
      </tr>
      <tr>
        <td id="L428" class="blob-num js-line-number" data-line-number="428"></td>
        <td id="LC428" class="blob-code js-file-line"><span class="kd">function</span> <span class="nx">cubism_graphiteParse</span><span class="p">(</span><span class="nx">text</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L429" class="blob-num js-line-number" data-line-number="429"></td>
        <td id="LC429" class="blob-code js-file-line">  <span class="kd">var</span> <span class="nx">i</span> <span class="o">=</span> <span class="nx">text</span><span class="p">.</span><span class="nx">indexOf</span><span class="p">(</span><span class="s2">&quot;|&quot;</span><span class="p">),</span></td>
      </tr>
      <tr>
        <td id="L430" class="blob-num js-line-number" data-line-number="430"></td>
        <td id="LC430" class="blob-code js-file-line">      <span class="nx">meta</span> <span class="o">=</span> <span class="nx">text</span><span class="p">.</span><span class="nx">substring</span><span class="p">(</span><span class="mi">0</span><span class="p">,</span> <span class="nx">i</span><span class="p">),</span></td>
      </tr>
      <tr>
        <td id="L431" class="blob-num js-line-number" data-line-number="431"></td>
        <td id="LC431" class="blob-code js-file-line">      <span class="nx">c</span> <span class="o">=</span> <span class="nx">meta</span><span class="p">.</span><span class="nx">lastIndexOf</span><span class="p">(</span><span class="s2">&quot;,&quot;</span><span class="p">),</span></td>
      </tr>
      <tr>
        <td id="L432" class="blob-num js-line-number" data-line-number="432"></td>
        <td id="LC432" class="blob-code js-file-line">      <span class="nx">b</span> <span class="o">=</span> <span class="nx">meta</span><span class="p">.</span><span class="nx">lastIndexOf</span><span class="p">(</span><span class="s2">&quot;,&quot;</span><span class="p">,</span> <span class="nx">c</span> <span class="o">-</span> <span class="mi">1</span><span class="p">),</span></td>
      </tr>
      <tr>
        <td id="L433" class="blob-num js-line-number" data-line-number="433"></td>
        <td id="LC433" class="blob-code js-file-line">      <span class="nx">a</span> <span class="o">=</span> <span class="nx">meta</span><span class="p">.</span><span class="nx">lastIndexOf</span><span class="p">(</span><span class="s2">&quot;,&quot;</span><span class="p">,</span> <span class="nx">b</span> <span class="o">-</span> <span class="mi">1</span><span class="p">),</span></td>
      </tr>
      <tr>
        <td id="L434" class="blob-num js-line-number" data-line-number="434"></td>
        <td id="LC434" class="blob-code js-file-line">      <span class="nx">start</span> <span class="o">=</span> <span class="nx">meta</span><span class="p">.</span><span class="nx">substring</span><span class="p">(</span><span class="nx">a</span> <span class="o">+</span> <span class="mi">1</span><span class="p">,</span> <span class="nx">b</span><span class="p">)</span> <span class="o">*</span> <span class="mi">1000</span><span class="p">,</span></td>
      </tr>
      <tr>
        <td id="L435" class="blob-num js-line-number" data-line-number="435"></td>
        <td id="LC435" class="blob-code js-file-line">      <span class="nx">step</span> <span class="o">=</span> <span class="nx">meta</span><span class="p">.</span><span class="nx">substring</span><span class="p">(</span><span class="nx">c</span> <span class="o">+</span> <span class="mi">1</span><span class="p">)</span> <span class="o">*</span> <span class="mi">1000</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L436" class="blob-num js-line-number" data-line-number="436"></td>
        <td id="LC436" class="blob-code js-file-line">  <span class="k">return</span> <span class="nx">text</span></td>
      </tr>
      <tr>
        <td id="L437" class="blob-num js-line-number" data-line-number="437"></td>
        <td id="LC437" class="blob-code js-file-line">      <span class="p">.</span><span class="nx">substring</span><span class="p">(</span><span class="nx">i</span> <span class="o">+</span> <span class="mi">1</span><span class="p">)</span></td>
      </tr>
      <tr>
        <td id="L438" class="blob-num js-line-number" data-line-number="438"></td>
        <td id="LC438" class="blob-code js-file-line">      <span class="p">.</span><span class="nx">split</span><span class="p">(</span><span class="s2">&quot;,&quot;</span><span class="p">)</span></td>
      </tr>
      <tr>
        <td id="L439" class="blob-num js-line-number" data-line-number="439"></td>
        <td id="LC439" class="blob-code js-file-line">      <span class="p">.</span><span class="nx">slice</span><span class="p">(</span><span class="mi">1</span><span class="p">)</span> <span class="c1">// the first value is always None?</span></td>
      </tr>
      <tr>
        <td id="L440" class="blob-num js-line-number" data-line-number="440"></td>
        <td id="LC440" class="blob-code js-file-line">      <span class="p">.</span><span class="nx">map</span><span class="p">(</span><span class="kd">function</span><span class="p">(</span><span class="nx">d</span><span class="p">)</span> <span class="p">{</span> <span class="k">return</span> <span class="o">+</span><span class="nx">d</span><span class="p">;</span> <span class="p">});</span></td>
      </tr>
      <tr>
        <td id="L441" class="blob-num js-line-number" data-line-number="441"></td>
        <td id="LC441" class="blob-code js-file-line"><span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L442" class="blob-num js-line-number" data-line-number="442"></td>
        <td id="LC442" class="blob-code js-file-line"><span class="nx">cubism_contextPrototype</span><span class="p">.</span><span class="nx">gangliaWeb</span> <span class="o">=</span> <span class="kd">function</span><span class="p">(</span><span class="nx">config</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L443" class="blob-num js-line-number" data-line-number="443"></td>
        <td id="LC443" class="blob-code js-file-line">  <span class="kd">var</span> <span class="nx">host</span> <span class="o">=</span> <span class="s1">&#39;&#39;</span><span class="p">,</span></td>
      </tr>
      <tr>
        <td id="L444" class="blob-num js-line-number" data-line-number="444"></td>
        <td id="LC444" class="blob-code js-file-line">      <span class="nx">uriPathPrefix</span> <span class="o">=</span> <span class="s1">&#39;/ganglia2/&#39;</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L445" class="blob-num js-line-number" data-line-number="445"></td>
        <td id="LC445" class="blob-code js-file-line"> </td>
      </tr>
      <tr>
        <td id="L446" class="blob-num js-line-number" data-line-number="446"></td>
        <td id="LC446" class="blob-code js-file-line">  <span class="k">if</span> <span class="p">(</span><span class="nx">arguments</span><span class="p">.</span><span class="nx">length</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L447" class="blob-num js-line-number" data-line-number="447"></td>
        <td id="LC447" class="blob-code js-file-line">    <span class="k">if</span> <span class="p">(</span><span class="nx">config</span><span class="p">.</span><span class="nx">host</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L448" class="blob-num js-line-number" data-line-number="448"></td>
        <td id="LC448" class="blob-code js-file-line">      <span class="nx">host</span> <span class="o">=</span> <span class="nx">config</span><span class="p">.</span><span class="nx">host</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L449" class="blob-num js-line-number" data-line-number="449"></td>
        <td id="LC449" class="blob-code js-file-line">    <span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L450" class="blob-num js-line-number" data-line-number="450"></td>
        <td id="LC450" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L451" class="blob-num js-line-number" data-line-number="451"></td>
        <td id="LC451" class="blob-code js-file-line">    <span class="k">if</span> <span class="p">(</span><span class="nx">config</span><span class="p">.</span><span class="nx">uriPathPrefix</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L452" class="blob-num js-line-number" data-line-number="452"></td>
        <td id="LC452" class="blob-code js-file-line">      <span class="nx">uriPathPrefix</span> <span class="o">=</span> <span class="nx">config</span><span class="p">.</span><span class="nx">uriPathPrefix</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L453" class="blob-num js-line-number" data-line-number="453"></td>
        <td id="LC453" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L454" class="blob-num js-line-number" data-line-number="454"></td>
        <td id="LC454" class="blob-code js-file-line">      <span class="cm">/* Add leading and trailing slashes, as appropriate. */</span></td>
      </tr>
      <tr>
        <td id="L455" class="blob-num js-line-number" data-line-number="455"></td>
        <td id="LC455" class="blob-code js-file-line">      <span class="k">if</span><span class="p">(</span> <span class="nx">uriPathPrefix</span><span class="p">[</span><span class="mi">0</span><span class="p">]</span> <span class="o">!=</span> <span class="s1">&#39;/&#39;</span> <span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L456" class="blob-num js-line-number" data-line-number="456"></td>
        <td id="LC456" class="blob-code js-file-line">        <span class="nx">uriPathPrefix</span> <span class="o">=</span> <span class="s1">&#39;/&#39;</span> <span class="o">+</span> <span class="nx">uriPathPrefix</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L457" class="blob-num js-line-number" data-line-number="457"></td>
        <td id="LC457" class="blob-code js-file-line">      <span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L458" class="blob-num js-line-number" data-line-number="458"></td>
        <td id="LC458" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L459" class="blob-num js-line-number" data-line-number="459"></td>
        <td id="LC459" class="blob-code js-file-line">      <span class="k">if</span><span class="p">(</span> <span class="nx">uriPathPrefix</span><span class="p">[</span><span class="nx">uriPathPrefix</span><span class="p">.</span><span class="nx">length</span> <span class="o">-</span> <span class="mi">1</span><span class="p">]</span> <span class="o">!=</span> <span class="s1">&#39;/&#39;</span> <span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L460" class="blob-num js-line-number" data-line-number="460"></td>
        <td id="LC460" class="blob-code js-file-line">        <span class="nx">uriPathPrefix</span> <span class="o">+=</span> <span class="s1">&#39;/&#39;</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L461" class="blob-num js-line-number" data-line-number="461"></td>
        <td id="LC461" class="blob-code js-file-line">      <span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L462" class="blob-num js-line-number" data-line-number="462"></td>
        <td id="LC462" class="blob-code js-file-line">    <span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L463" class="blob-num js-line-number" data-line-number="463"></td>
        <td id="LC463" class="blob-code js-file-line">  <span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L464" class="blob-num js-line-number" data-line-number="464"></td>
        <td id="LC464" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L465" class="blob-num js-line-number" data-line-number="465"></td>
        <td id="LC465" class="blob-code js-file-line">  <span class="kd">var</span> <span class="nx">source</span> <span class="o">=</span> <span class="p">{},</span></td>
      </tr>
      <tr>
        <td id="L466" class="blob-num js-line-number" data-line-number="466"></td>
        <td id="LC466" class="blob-code js-file-line">      <span class="nx">context</span> <span class="o">=</span> <span class="k">this</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L467" class="blob-num js-line-number" data-line-number="467"></td>
        <td id="LC467" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L468" class="blob-num js-line-number" data-line-number="468"></td>
        <td id="LC468" class="blob-code js-file-line">  <span class="nx">source</span><span class="p">.</span><span class="nx">metric</span> <span class="o">=</span> <span class="kd">function</span><span class="p">(</span><span class="nx">metricInfo</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L469" class="blob-num js-line-number" data-line-number="469"></td>
        <td id="LC469" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L470" class="blob-num js-line-number" data-line-number="470"></td>
        <td id="LC470" class="blob-code js-file-line">    <span class="cm">/* Store the members from metricInfo into local variables. */</span></td>
      </tr>
      <tr>
        <td id="L471" class="blob-num js-line-number" data-line-number="471"></td>
        <td id="LC471" class="blob-code js-file-line">    <span class="kd">var</span> <span class="nx">clusterName</span> <span class="o">=</span> <span class="nx">metricInfo</span><span class="p">.</span><span class="nx">clusterName</span><span class="p">,</span> </td>
      </tr>
      <tr>
        <td id="L472" class="blob-num js-line-number" data-line-number="472"></td>
        <td id="LC472" class="blob-code js-file-line">        <span class="nx">metricName</span> <span class="o">=</span> <span class="nx">metricInfo</span><span class="p">.</span><span class="nx">metricName</span><span class="p">,</span> </td>
      </tr>
      <tr>
        <td id="L473" class="blob-num js-line-number" data-line-number="473"></td>
        <td id="LC473" class="blob-code js-file-line">        <span class="nx">hostName</span> <span class="o">=</span> <span class="nx">metricInfo</span><span class="p">.</span><span class="nx">hostName</span><span class="p">,</span></td>
      </tr>
      <tr>
        <td id="L474" class="blob-num js-line-number" data-line-number="474"></td>
        <td id="LC474" class="blob-code js-file-line">        <span class="nx">isReport</span> <span class="o">=</span> <span class="nx">metricInfo</span><span class="p">.</span><span class="nx">isReport</span> <span class="o">||</span> <span class="kc">false</span><span class="p">,</span></td>
      </tr>
      <tr>
        <td id="L475" class="blob-num js-line-number" data-line-number="475"></td>
        <td id="LC475" class="blob-code js-file-line">        <span class="nx">titleGenerator</span> <span class="o">=</span> <span class="nx">metricInfo</span><span class="p">.</span><span class="nx">titleGenerator</span> <span class="o">||</span></td>
      </tr>
      <tr>
        <td id="L476" class="blob-num js-line-number" data-line-number="476"></td>
        <td id="LC476" class="blob-code js-file-line">          <span class="cm">/* Reasonable (not necessarily pretty) default for titleGenerator. */</span></td>
      </tr>
      <tr>
        <td id="L477" class="blob-num js-line-number" data-line-number="477"></td>
        <td id="LC477" class="blob-code js-file-line">          <span class="kd">function</span><span class="p">(</span><span class="nx">unusedMetricInfo</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L478" class="blob-num js-line-number" data-line-number="478"></td>
        <td id="LC478" class="blob-code js-file-line">            <span class="cm">/* unusedMetricInfo is, well, unused in this default case. */</span></td>
      </tr>
      <tr>
        <td id="L479" class="blob-num js-line-number" data-line-number="479"></td>
        <td id="LC479" class="blob-code js-file-line">            <span class="k">return</span> <span class="p">(</span><span class="s1">&#39;clusterName:&#39;</span> <span class="o">+</span> <span class="nx">clusterName</span> <span class="o">+</span> </td>
      </tr>
      <tr>
        <td id="L480" class="blob-num js-line-number" data-line-number="480"></td>
        <td id="LC480" class="blob-code js-file-line">                    <span class="s1">&#39; metricName:&#39;</span> <span class="o">+</span> <span class="nx">metricName</span> <span class="o">+</span></td>
      </tr>
      <tr>
        <td id="L481" class="blob-num js-line-number" data-line-number="481"></td>
        <td id="LC481" class="blob-code js-file-line">                    <span class="p">(</span><span class="nx">hostName</span> <span class="o">?</span> <span class="s1">&#39; hostName:&#39;</span> <span class="o">+</span> <span class="nx">hostName</span> <span class="o">:</span> <span class="s1">&#39;&#39;</span><span class="p">));</span></td>
      </tr>
      <tr>
        <td id="L482" class="blob-num js-line-number" data-line-number="482"></td>
        <td id="LC482" class="blob-code js-file-line">          <span class="p">},</span></td>
      </tr>
      <tr>
        <td id="L483" class="blob-num js-line-number" data-line-number="483"></td>
        <td id="LC483" class="blob-code js-file-line">        <span class="nx">onChangeCallback</span> <span class="o">=</span> <span class="nx">metricInfo</span><span class="p">.</span><span class="nx">onChangeCallback</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L484" class="blob-num js-line-number" data-line-number="484"></td>
        <td id="LC484" class="blob-code js-file-line">    </td>
      </tr>
      <tr>
        <td id="L485" class="blob-num js-line-number" data-line-number="485"></td>
        <td id="LC485" class="blob-code js-file-line">    <span class="cm">/* Default to plain, simple metrics. */</span></td>
      </tr>
      <tr>
        <td id="L486" class="blob-num js-line-number" data-line-number="486"></td>
        <td id="LC486" class="blob-code js-file-line">    <span class="kd">var</span> <span class="nx">metricKeyName</span> <span class="o">=</span> <span class="nx">isReport</span> <span class="o">?</span> <span class="s1">&#39;g&#39;</span> <span class="o">:</span> <span class="s1">&#39;m&#39;</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L487" class="blob-num js-line-number" data-line-number="487"></td>
        <td id="LC487" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L488" class="blob-num js-line-number" data-line-number="488"></td>
        <td id="LC488" class="blob-code js-file-line">    <span class="kd">var</span> <span class="nx">gangliaWebMetric</span> <span class="o">=</span> <span class="nx">context</span><span class="p">.</span><span class="nx">metric</span><span class="p">(</span><span class="kd">function</span><span class="p">(</span><span class="nx">start</span><span class="p">,</span> <span class="nx">stop</span><span class="p">,</span> <span class="nx">step</span><span class="p">,</span> <span class="nx">callback</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L489" class="blob-num js-line-number" data-line-number="489"></td>
        <td id="LC489" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L490" class="blob-num js-line-number" data-line-number="490"></td>
        <td id="LC490" class="blob-code js-file-line">      <span class="kd">function</span> <span class="nx">constructGangliaWebRequestQueryParams</span><span class="p">()</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L491" class="blob-num js-line-number" data-line-number="491"></td>
        <td id="LC491" class="blob-code js-file-line">        <span class="k">return</span> <span class="p">(</span><span class="s1">&#39;c=&#39;</span> <span class="o">+</span> <span class="nx">clusterName</span> <span class="o">+</span></td>
      </tr>
      <tr>
        <td id="L492" class="blob-num js-line-number" data-line-number="492"></td>
        <td id="LC492" class="blob-code js-file-line">                <span class="s1">&#39;&amp;&#39;</span> <span class="o">+</span> <span class="nx">metricKeyName</span> <span class="o">+</span> <span class="s1">&#39;=&#39;</span> <span class="o">+</span> <span class="nx">metricName</span> <span class="o">+</span> </td>
      </tr>
      <tr>
        <td id="L493" class="blob-num js-line-number" data-line-number="493"></td>
        <td id="LC493" class="blob-code js-file-line">                <span class="p">(</span><span class="nx">hostName</span> <span class="o">?</span> <span class="s1">&#39;&amp;h=&#39;</span> <span class="o">+</span> <span class="nx">hostName</span> <span class="o">:</span> <span class="s1">&#39;&#39;</span><span class="p">)</span> <span class="o">+</span> </td>
      </tr>
      <tr>
        <td id="L494" class="blob-num js-line-number" data-line-number="494"></td>
        <td id="LC494" class="blob-code js-file-line">                <span class="s1">&#39;&amp;cs=&#39;</span> <span class="o">+</span> <span class="nx">start</span><span class="o">/</span><span class="mi">1000</span> <span class="o">+</span> <span class="s1">&#39;&amp;ce=&#39;</span> <span class="o">+</span> <span class="nx">stop</span><span class="o">/</span><span class="mi">1000</span> <span class="o">+</span> <span class="s1">&#39;&amp;step=&#39;</span> <span class="o">+</span> <span class="nx">step</span><span class="o">/</span><span class="mi">1000</span> <span class="o">+</span> <span class="s1">&#39;&amp;graphlot=1&#39;</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L495" class="blob-num js-line-number" data-line-number="495"></td>
        <td id="LC495" class="blob-code js-file-line">      <span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L496" class="blob-num js-line-number" data-line-number="496"></td>
        <td id="LC496" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L497" class="blob-num js-line-number" data-line-number="497"></td>
        <td id="LC497" class="blob-code js-file-line">      <span class="nx">d3</span><span class="p">.</span><span class="nx">json</span><span class="p">(</span><span class="nx">host</span> <span class="o">+</span> <span class="nx">uriPathPrefix</span> <span class="o">+</span> <span class="s1">&#39;graph.php?&#39;</span> <span class="o">+</span> <span class="nx">constructGangliaWebRequestQueryParams</span><span class="p">(),</span></td>
      </tr>
      <tr>
        <td id="L498" class="blob-num js-line-number" data-line-number="498"></td>
        <td id="LC498" class="blob-code js-file-line">        <span class="kd">function</span><span class="p">(</span><span class="nx">result</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L499" class="blob-num js-line-number" data-line-number="499"></td>
        <td id="LC499" class="blob-code js-file-line">          <span class="k">if</span><span class="p">(</span> <span class="o">!</span><span class="nx">result</span> <span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L500" class="blob-num js-line-number" data-line-number="500"></td>
        <td id="LC500" class="blob-code js-file-line">            <span class="k">return</span> <span class="nx">callback</span><span class="p">(</span><span class="k">new</span> <span class="nb">Error</span><span class="p">(</span><span class="s2">&quot;Unable to fetch GangliaWeb data&quot;</span><span class="p">));</span></td>
      </tr>
      <tr>
        <td id="L501" class="blob-num js-line-number" data-line-number="501"></td>
        <td id="LC501" class="blob-code js-file-line">          <span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L502" class="blob-num js-line-number" data-line-number="502"></td>
        <td id="LC502" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L503" class="blob-num js-line-number" data-line-number="503"></td>
        <td id="LC503" class="blob-code js-file-line">          <span class="nx">callback</span><span class="p">(</span><span class="kc">null</span><span class="p">,</span> <span class="nx">result</span><span class="p">[</span><span class="mi">0</span><span class="p">].</span><span class="nx">data</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L504" class="blob-num js-line-number" data-line-number="504"></td>
        <td id="LC504" class="blob-code js-file-line">        <span class="p">});</span></td>
      </tr>
      <tr>
        <td id="L505" class="blob-num js-line-number" data-line-number="505"></td>
        <td id="LC505" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L506" class="blob-num js-line-number" data-line-number="506"></td>
        <td id="LC506" class="blob-code js-file-line">    <span class="p">},</span> <span class="nx">titleGenerator</span><span class="p">(</span><span class="nx">metricInfo</span><span class="p">));</span></td>
      </tr>
      <tr>
        <td id="L507" class="blob-num js-line-number" data-line-number="507"></td>
        <td id="LC507" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L508" class="blob-num js-line-number" data-line-number="508"></td>
        <td id="LC508" class="blob-code js-file-line">    <span class="nx">gangliaWebMetric</span><span class="p">.</span><span class="nx">toString</span> <span class="o">=</span> <span class="kd">function</span><span class="p">()</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L509" class="blob-num js-line-number" data-line-number="509"></td>
        <td id="LC509" class="blob-code js-file-line">      <span class="k">return</span> <span class="nx">titleGenerator</span><span class="p">(</span><span class="nx">metricInfo</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L510" class="blob-num js-line-number" data-line-number="510"></td>
        <td id="LC510" class="blob-code js-file-line">    <span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L511" class="blob-num js-line-number" data-line-number="511"></td>
        <td id="LC511" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L512" class="blob-num js-line-number" data-line-number="512"></td>
        <td id="LC512" class="blob-code js-file-line">    <span class="cm">/* Allow users to run their custom code each time a gangliaWebMetric changes.</span></td>
      </tr>
      <tr>
        <td id="L513" class="blob-num js-line-number" data-line-number="513"></td>
        <td id="LC513" class="blob-code js-file-line"><span class="cm">     *</span></td>
      </tr>
      <tr>
        <td id="L514" class="blob-num js-line-number" data-line-number="514"></td>
        <td id="LC514" class="blob-code js-file-line"><span class="cm">     * TODO Consider abstracting away the naked Cubism call, and instead exposing </span></td>
      </tr>
      <tr>
        <td id="L515" class="blob-num js-line-number" data-line-number="515"></td>
        <td id="LC515" class="blob-code js-file-line"><span class="cm">     * a callback that takes in the values array (maybe alongwith the original</span></td>
      </tr>
      <tr>
        <td id="L516" class="blob-num js-line-number" data-line-number="516"></td>
        <td id="LC516" class="blob-code js-file-line"><span class="cm">     * start and stop &#39;naked&#39; parameters), since it&#39;s handy to have the entire</span></td>
      </tr>
      <tr>
        <td id="L517" class="blob-num js-line-number" data-line-number="517"></td>
        <td id="LC517" class="blob-code js-file-line"><span class="cm">     * dataset at your disposal (and users will likely implement onChangeCallback</span></td>
      </tr>
      <tr>
        <td id="L518" class="blob-num js-line-number" data-line-number="518"></td>
        <td id="LC518" class="blob-code js-file-line"><span class="cm">     * primarily to get at this dataset).</span></td>
      </tr>
      <tr>
        <td id="L519" class="blob-num js-line-number" data-line-number="519"></td>
        <td id="LC519" class="blob-code js-file-line"><span class="cm">     */</span></td>
      </tr>
      <tr>
        <td id="L520" class="blob-num js-line-number" data-line-number="520"></td>
        <td id="LC520" class="blob-code js-file-line">    <span class="k">if</span> <span class="p">(</span><span class="nx">onChangeCallback</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L521" class="blob-num js-line-number" data-line-number="521"></td>
        <td id="LC521" class="blob-code js-file-line">      <span class="nx">gangliaWebMetric</span><span class="p">.</span><span class="nx">on</span><span class="p">(</span><span class="s1">&#39;change&#39;</span><span class="p">,</span> <span class="nx">onChangeCallback</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L522" class="blob-num js-line-number" data-line-number="522"></td>
        <td id="LC522" class="blob-code js-file-line">    <span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L523" class="blob-num js-line-number" data-line-number="523"></td>
        <td id="LC523" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L524" class="blob-num js-line-number" data-line-number="524"></td>
        <td id="LC524" class="blob-code js-file-line">    <span class="k">return</span> <span class="nx">gangliaWebMetric</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L525" class="blob-num js-line-number" data-line-number="525"></td>
        <td id="LC525" class="blob-code js-file-line">  <span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L526" class="blob-num js-line-number" data-line-number="526"></td>
        <td id="LC526" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L527" class="blob-num js-line-number" data-line-number="527"></td>
        <td id="LC527" class="blob-code js-file-line">  <span class="c1">// Returns the gangliaWeb host + uriPathPrefix.</span></td>
      </tr>
      <tr>
        <td id="L528" class="blob-num js-line-number" data-line-number="528"></td>
        <td id="LC528" class="blob-code js-file-line">  <span class="nx">source</span><span class="p">.</span><span class="nx">toString</span> <span class="o">=</span> <span class="kd">function</span><span class="p">()</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L529" class="blob-num js-line-number" data-line-number="529"></td>
        <td id="LC529" class="blob-code js-file-line">    <span class="k">return</span> <span class="nx">host</span> <span class="o">+</span> <span class="nx">uriPathPrefix</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L530" class="blob-num js-line-number" data-line-number="530"></td>
        <td id="LC530" class="blob-code js-file-line">  <span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L531" class="blob-num js-line-number" data-line-number="531"></td>
        <td id="LC531" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L532" class="blob-num js-line-number" data-line-number="532"></td>
        <td id="LC532" class="blob-code js-file-line">  <span class="k">return</span> <span class="nx">source</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L533" class="blob-num js-line-number" data-line-number="533"></td>
        <td id="LC533" class="blob-code js-file-line"><span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L534" class="blob-num js-line-number" data-line-number="534"></td>
        <td id="LC534" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L535" class="blob-num js-line-number" data-line-number="535"></td>
        <td id="LC535" class="blob-code js-file-line"><span class="kd">function</span> <span class="nx">cubism_metric</span><span class="p">(</span><span class="nx">context</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L536" class="blob-num js-line-number" data-line-number="536"></td>
        <td id="LC536" class="blob-code js-file-line">  <span class="k">if</span> <span class="p">(</span><span class="o">!</span><span class="p">(</span><span class="nx">context</span> <span class="k">instanceof</span> <span class="nx">cubism_context</span><span class="p">))</span> <span class="k">throw</span> <span class="k">new</span> <span class="nb">Error</span><span class="p">(</span><span class="s2">&quot;invalid context&quot;</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L537" class="blob-num js-line-number" data-line-number="537"></td>
        <td id="LC537" class="blob-code js-file-line">  <span class="k">this</span><span class="p">.</span><span class="nx">context</span> <span class="o">=</span> <span class="nx">context</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L538" class="blob-num js-line-number" data-line-number="538"></td>
        <td id="LC538" class="blob-code js-file-line"><span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L539" class="blob-num js-line-number" data-line-number="539"></td>
        <td id="LC539" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L540" class="blob-num js-line-number" data-line-number="540"></td>
        <td id="LC540" class="blob-code js-file-line"><span class="kd">var</span> <span class="nx">cubism_metricPrototype</span> <span class="o">=</span> <span class="nx">cubism_metric</span><span class="p">.</span><span class="nx">prototype</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L541" class="blob-num js-line-number" data-line-number="541"></td>
        <td id="LC541" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L542" class="blob-num js-line-number" data-line-number="542"></td>
        <td id="LC542" class="blob-code js-file-line"><span class="nx">cubism</span><span class="p">.</span><span class="nx">metric</span> <span class="o">=</span> <span class="nx">cubism_metric</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L543" class="blob-num js-line-number" data-line-number="543"></td>
        <td id="LC543" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L544" class="blob-num js-line-number" data-line-number="544"></td>
        <td id="LC544" class="blob-code js-file-line"><span class="nx">cubism_metricPrototype</span><span class="p">.</span><span class="nx">valueAt</span> <span class="o">=</span> <span class="kd">function</span><span class="p">()</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L545" class="blob-num js-line-number" data-line-number="545"></td>
        <td id="LC545" class="blob-code js-file-line">  <span class="k">return</span> <span class="kc">NaN</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L546" class="blob-num js-line-number" data-line-number="546"></td>
        <td id="LC546" class="blob-code js-file-line"><span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L547" class="blob-num js-line-number" data-line-number="547"></td>
        <td id="LC547" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L548" class="blob-num js-line-number" data-line-number="548"></td>
        <td id="LC548" class="blob-code js-file-line"><span class="nx">cubism_metricPrototype</span><span class="p">.</span><span class="nx">alias</span> <span class="o">=</span> <span class="kd">function</span><span class="p">(</span><span class="nx">name</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L549" class="blob-num js-line-number" data-line-number="549"></td>
        <td id="LC549" class="blob-code js-file-line">  <span class="k">this</span><span class="p">.</span><span class="nx">toString</span> <span class="o">=</span> <span class="kd">function</span><span class="p">()</span> <span class="p">{</span> <span class="k">return</span> <span class="nx">name</span><span class="p">;</span> <span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L550" class="blob-num js-line-number" data-line-number="550"></td>
        <td id="LC550" class="blob-code js-file-line">  <span class="k">return</span> <span class="k">this</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L551" class="blob-num js-line-number" data-line-number="551"></td>
        <td id="LC551" class="blob-code js-file-line"><span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L552" class="blob-num js-line-number" data-line-number="552"></td>
        <td id="LC552" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L553" class="blob-num js-line-number" data-line-number="553"></td>
        <td id="LC553" class="blob-code js-file-line"><span class="nx">cubism_metricPrototype</span><span class="p">.</span><span class="nx">extent</span> <span class="o">=</span> <span class="kd">function</span><span class="p">()</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L554" class="blob-num js-line-number" data-line-number="554"></td>
        <td id="LC554" class="blob-code js-file-line">  <span class="kd">var</span> <span class="nx">i</span> <span class="o">=</span> <span class="mi">0</span><span class="p">,</span></td>
      </tr>
      <tr>
        <td id="L555" class="blob-num js-line-number" data-line-number="555"></td>
        <td id="LC555" class="blob-code js-file-line">      <span class="nx">n</span> <span class="o">=</span> <span class="k">this</span><span class="p">.</span><span class="nx">context</span><span class="p">.</span><span class="nx">size</span><span class="p">(),</span></td>
      </tr>
      <tr>
        <td id="L556" class="blob-num js-line-number" data-line-number="556"></td>
        <td id="LC556" class="blob-code js-file-line">      <span class="nx">value</span><span class="p">,</span></td>
      </tr>
      <tr>
        <td id="L557" class="blob-num js-line-number" data-line-number="557"></td>
        <td id="LC557" class="blob-code js-file-line">      <span class="nx">min</span> <span class="o">=</span> <span class="kc">Infinity</span><span class="p">,</span></td>
      </tr>
      <tr>
        <td id="L558" class="blob-num js-line-number" data-line-number="558"></td>
        <td id="LC558" class="blob-code js-file-line">      <span class="nx">max</span> <span class="o">=</span> <span class="o">-</span><span class="kc">Infinity</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L559" class="blob-num js-line-number" data-line-number="559"></td>
        <td id="LC559" class="blob-code js-file-line">  <span class="k">while</span> <span class="p">(</span><span class="o">++</span><span class="nx">i</span> <span class="o">&lt;</span> <span class="nx">n</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L560" class="blob-num js-line-number" data-line-number="560"></td>
        <td id="LC560" class="blob-code js-file-line">    <span class="nx">value</span> <span class="o">=</span> <span class="k">this</span><span class="p">.</span><span class="nx">valueAt</span><span class="p">(</span><span class="nx">i</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L561" class="blob-num js-line-number" data-line-number="561"></td>
        <td id="LC561" class="blob-code js-file-line">    <span class="k">if</span> <span class="p">(</span><span class="nx">value</span> <span class="o">&lt;</span> <span class="nx">min</span><span class="p">)</span> <span class="nx">min</span> <span class="o">=</span> <span class="nx">value</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L562" class="blob-num js-line-number" data-line-number="562"></td>
        <td id="LC562" class="blob-code js-file-line">    <span class="k">if</span> <span class="p">(</span><span class="nx">value</span> <span class="o">&gt;</span> <span class="nx">max</span><span class="p">)</span> <span class="nx">max</span> <span class="o">=</span> <span class="nx">value</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L563" class="blob-num js-line-number" data-line-number="563"></td>
        <td id="LC563" class="blob-code js-file-line">  <span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L564" class="blob-num js-line-number" data-line-number="564"></td>
        <td id="LC564" class="blob-code js-file-line">  <span class="k">return</span> <span class="p">[</span><span class="nx">min</span><span class="p">,</span> <span class="nx">max</span><span class="p">];</span></td>
      </tr>
      <tr>
        <td id="L565" class="blob-num js-line-number" data-line-number="565"></td>
        <td id="LC565" class="blob-code js-file-line"><span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L566" class="blob-num js-line-number" data-line-number="566"></td>
        <td id="LC566" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L567" class="blob-num js-line-number" data-line-number="567"></td>
        <td id="LC567" class="blob-code js-file-line"><span class="nx">cubism_metricPrototype</span><span class="p">.</span><span class="nx">on</span> <span class="o">=</span> <span class="kd">function</span><span class="p">(</span><span class="nx">type</span><span class="p">,</span> <span class="nx">listener</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L568" class="blob-num js-line-number" data-line-number="568"></td>
        <td id="LC568" class="blob-code js-file-line">  <span class="k">return</span> <span class="nx">arguments</span><span class="p">.</span><span class="nx">length</span> <span class="o">&lt;</span> <span class="mi">2</span> <span class="o">?</span> <span class="kc">null</span> <span class="o">:</span> <span class="k">this</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L569" class="blob-num js-line-number" data-line-number="569"></td>
        <td id="LC569" class="blob-code js-file-line"><span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L570" class="blob-num js-line-number" data-line-number="570"></td>
        <td id="LC570" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L571" class="blob-num js-line-number" data-line-number="571"></td>
        <td id="LC571" class="blob-code js-file-line"><span class="nx">cubism_metricPrototype</span><span class="p">.</span><span class="nx">shift</span> <span class="o">=</span> <span class="kd">function</span><span class="p">()</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L572" class="blob-num js-line-number" data-line-number="572"></td>
        <td id="LC572" class="blob-code js-file-line">  <span class="k">return</span> <span class="k">this</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L573" class="blob-num js-line-number" data-line-number="573"></td>
        <td id="LC573" class="blob-code js-file-line"><span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L574" class="blob-num js-line-number" data-line-number="574"></td>
        <td id="LC574" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L575" class="blob-num js-line-number" data-line-number="575"></td>
        <td id="LC575" class="blob-code js-file-line"><span class="nx">cubism_metricPrototype</span><span class="p">.</span><span class="nx">on</span> <span class="o">=</span> <span class="kd">function</span><span class="p">()</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L576" class="blob-num js-line-number" data-line-number="576"></td>
        <td id="LC576" class="blob-code js-file-line">  <span class="k">return</span> <span class="nx">arguments</span><span class="p">.</span><span class="nx">length</span> <span class="o">&lt;</span> <span class="mi">2</span> <span class="o">?</span> <span class="kc">null</span> <span class="o">:</span> <span class="k">this</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L577" class="blob-num js-line-number" data-line-number="577"></td>
        <td id="LC577" class="blob-code js-file-line"><span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L578" class="blob-num js-line-number" data-line-number="578"></td>
        <td id="LC578" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L579" class="blob-num js-line-number" data-line-number="579"></td>
        <td id="LC579" class="blob-code js-file-line"><span class="nx">cubism_contextPrototype</span><span class="p">.</span><span class="nx">metric</span> <span class="o">=</span> <span class="kd">function</span><span class="p">(</span><span class="nx">request</span><span class="p">,</span> <span class="nx">name</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L580" class="blob-num js-line-number" data-line-number="580"></td>
        <td id="LC580" class="blob-code js-file-line">  <span class="kd">var</span> <span class="nx">context</span> <span class="o">=</span> <span class="k">this</span><span class="p">,</span></td>
      </tr>
      <tr>
        <td id="L581" class="blob-num js-line-number" data-line-number="581"></td>
        <td id="LC581" class="blob-code js-file-line">      <span class="nx">metric</span> <span class="o">=</span> <span class="k">new</span> <span class="nx">cubism_metric</span><span class="p">(</span><span class="nx">context</span><span class="p">),</span></td>
      </tr>
      <tr>
        <td id="L582" class="blob-num js-line-number" data-line-number="582"></td>
        <td id="LC582" class="blob-code js-file-line">      <span class="nx">id</span> <span class="o">=</span> <span class="s2">&quot;.metric-&quot;</span> <span class="o">+</span> <span class="o">++</span><span class="nx">cubism_id</span><span class="p">,</span></td>
      </tr>
      <tr>
        <td id="L583" class="blob-num js-line-number" data-line-number="583"></td>
        <td id="LC583" class="blob-code js-file-line">      <span class="nx">start</span> <span class="o">=</span> <span class="o">-</span><span class="kc">Infinity</span><span class="p">,</span></td>
      </tr>
      <tr>
        <td id="L584" class="blob-num js-line-number" data-line-number="584"></td>
        <td id="LC584" class="blob-code js-file-line">      <span class="nx">stop</span><span class="p">,</span></td>
      </tr>
      <tr>
        <td id="L585" class="blob-num js-line-number" data-line-number="585"></td>
        <td id="LC585" class="blob-code js-file-line">      <span class="nx">step</span> <span class="o">=</span> <span class="nx">context</span><span class="p">.</span><span class="nx">step</span><span class="p">(),</span></td>
      </tr>
      <tr>
        <td id="L586" class="blob-num js-line-number" data-line-number="586"></td>
        <td id="LC586" class="blob-code js-file-line">      <span class="nx">size</span> <span class="o">=</span> <span class="nx">context</span><span class="p">.</span><span class="nx">size</span><span class="p">(),</span></td>
      </tr>
      <tr>
        <td id="L587" class="blob-num js-line-number" data-line-number="587"></td>
        <td id="LC587" class="blob-code js-file-line">      <span class="nx">values</span> <span class="o">=</span> <span class="p">[],</span></td>
      </tr>
      <tr>
        <td id="L588" class="blob-num js-line-number" data-line-number="588"></td>
        <td id="LC588" class="blob-code js-file-line">      <span class="nx">event</span> <span class="o">=</span> <span class="nx">d3</span><span class="p">.</span><span class="nx">dispatch</span><span class="p">(</span><span class="s2">&quot;change&quot;</span><span class="p">),</span></td>
      </tr>
      <tr>
        <td id="L589" class="blob-num js-line-number" data-line-number="589"></td>
        <td id="LC589" class="blob-code js-file-line">      <span class="nx">listening</span> <span class="o">=</span> <span class="mi">0</span><span class="p">,</span></td>
      </tr>
      <tr>
        <td id="L590" class="blob-num js-line-number" data-line-number="590"></td>
        <td id="LC590" class="blob-code js-file-line">      <span class="nx">fetching</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L591" class="blob-num js-line-number" data-line-number="591"></td>
        <td id="LC591" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L592" class="blob-num js-line-number" data-line-number="592"></td>
        <td id="LC592" class="blob-code js-file-line">  <span class="c1">// Prefetch new data into a temporary array.</span></td>
      </tr>
      <tr>
        <td id="L593" class="blob-num js-line-number" data-line-number="593"></td>
        <td id="LC593" class="blob-code js-file-line">  <span class="kd">function</span> <span class="nx">prepare</span><span class="p">(</span><span class="nx">start1</span><span class="p">,</span> <span class="nx">stop</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L594" class="blob-num js-line-number" data-line-number="594"></td>
        <td id="LC594" class="blob-code js-file-line">    <span class="kd">var</span> <span class="nx">steps</span> <span class="o">=</span> <span class="nb">Math</span><span class="p">.</span><span class="nx">min</span><span class="p">(</span><span class="nx">size</span><span class="p">,</span> <span class="nb">Math</span><span class="p">.</span><span class="nx">round</span><span class="p">((</span><span class="nx">start1</span> <span class="o">-</span> <span class="nx">start</span><span class="p">)</span> <span class="o">/</span> <span class="nx">step</span><span class="p">));</span></td>
      </tr>
      <tr>
        <td id="L595" class="blob-num js-line-number" data-line-number="595"></td>
        <td id="LC595" class="blob-code js-file-line">    <span class="k">if</span> <span class="p">(</span><span class="o">!</span><span class="nx">steps</span> <span class="o">||</span> <span class="nx">fetching</span><span class="p">)</span> <span class="k">return</span><span class="p">;</span> <span class="c1">// already fetched, or fetching!</span></td>
      </tr>
      <tr>
        <td id="L596" class="blob-num js-line-number" data-line-number="596"></td>
        <td id="LC596" class="blob-code js-file-line">    <span class="nx">fetching</span> <span class="o">=</span> <span class="kc">true</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L597" class="blob-num js-line-number" data-line-number="597"></td>
        <td id="LC597" class="blob-code js-file-line">    <span class="nx">steps</span> <span class="o">=</span> <span class="nb">Math</span><span class="p">.</span><span class="nx">min</span><span class="p">(</span><span class="nx">size</span><span class="p">,</span> <span class="nx">steps</span> <span class="o">+</span> <span class="nx">cubism_metricOverlap</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L598" class="blob-num js-line-number" data-line-number="598"></td>
        <td id="LC598" class="blob-code js-file-line">    <span class="kd">var</span> <span class="nx">start0</span> <span class="o">=</span> <span class="k">new</span> <span class="nb">Date</span><span class="p">(</span><span class="nx">stop</span> <span class="o">-</span> <span class="nx">steps</span> <span class="o">*</span> <span class="nx">step</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L599" class="blob-num js-line-number" data-line-number="599"></td>
        <td id="LC599" class="blob-code js-file-line">    <span class="nx">request</span><span class="p">(</span><span class="nx">start0</span><span class="p">,</span> <span class="nx">stop</span><span class="p">,</span> <span class="nx">step</span><span class="p">,</span> <span class="kd">function</span><span class="p">(</span><span class="nx">error</span><span class="p">,</span> <span class="nx">data</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L600" class="blob-num js-line-number" data-line-number="600"></td>
        <td id="LC600" class="blob-code js-file-line">      <span class="nx">fetching</span> <span class="o">=</span> <span class="kc">false</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L601" class="blob-num js-line-number" data-line-number="601"></td>
        <td id="LC601" class="blob-code js-file-line">      <span class="k">if</span> <span class="p">(</span><span class="nx">error</span><span class="p">)</span> <span class="k">return</span> <span class="nx">console</span><span class="p">.</span><span class="nx">warn</span><span class="p">(</span><span class="nx">error</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L602" class="blob-num js-line-number" data-line-number="602"></td>
        <td id="LC602" class="blob-code js-file-line">      <span class="kd">var</span> <span class="nx">i</span> <span class="o">=</span> <span class="nb">isFinite</span><span class="p">(</span><span class="nx">start</span><span class="p">)</span> <span class="o">?</span> <span class="nb">Math</span><span class="p">.</span><span class="nx">round</span><span class="p">((</span><span class="nx">start0</span> <span class="o">-</span> <span class="nx">start</span><span class="p">)</span> <span class="o">/</span> <span class="nx">step</span><span class="p">)</span> <span class="o">:</span> <span class="mi">0</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L603" class="blob-num js-line-number" data-line-number="603"></td>
        <td id="LC603" class="blob-code js-file-line">      <span class="k">for</span> <span class="p">(</span><span class="kd">var</span> <span class="nx">j</span> <span class="o">=</span> <span class="mi">0</span><span class="p">,</span> <span class="nx">m</span> <span class="o">=</span> <span class="nx">data</span><span class="p">.</span><span class="nx">length</span><span class="p">;</span> <span class="nx">j</span> <span class="o">&lt;</span> <span class="nx">m</span><span class="p">;</span> <span class="o">++</span><span class="nx">j</span><span class="p">)</span> <span class="nx">values</span><span class="p">[</span><span class="nx">j</span> <span class="o">+</span> <span class="nx">i</span><span class="p">]</span> <span class="o">=</span> <span class="nx">data</span><span class="p">[</span><span class="nx">j</span><span class="p">];</span></td>
      </tr>
      <tr>
        <td id="L604" class="blob-num js-line-number" data-line-number="604"></td>
        <td id="LC604" class="blob-code js-file-line">      <span class="nx">event</span><span class="p">.</span><span class="nx">change</span><span class="p">.</span><span class="nx">call</span><span class="p">(</span><span class="nx">metric</span><span class="p">,</span> <span class="nx">start</span><span class="p">,</span> <span class="nx">stop</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L605" class="blob-num js-line-number" data-line-number="605"></td>
        <td id="LC605" class="blob-code js-file-line">    <span class="p">});</span></td>
      </tr>
      <tr>
        <td id="L606" class="blob-num js-line-number" data-line-number="606"></td>
        <td id="LC606" class="blob-code js-file-line">  <span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L607" class="blob-num js-line-number" data-line-number="607"></td>
        <td id="LC607" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L608" class="blob-num js-line-number" data-line-number="608"></td>
        <td id="LC608" class="blob-code js-file-line">  <span class="c1">// When the context changes, switch to the new data, ready-or-not!</span></td>
      </tr>
      <tr>
        <td id="L609" class="blob-num js-line-number" data-line-number="609"></td>
        <td id="LC609" class="blob-code js-file-line">  <span class="kd">function</span> <span class="nx">beforechange</span><span class="p">(</span><span class="nx">start1</span><span class="p">,</span> <span class="nx">stop1</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L610" class="blob-num js-line-number" data-line-number="610"></td>
        <td id="LC610" class="blob-code js-file-line">    <span class="k">if</span> <span class="p">(</span><span class="o">!</span><span class="nb">isFinite</span><span class="p">(</span><span class="nx">start</span><span class="p">))</span> <span class="nx">start</span> <span class="o">=</span> <span class="nx">start1</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L611" class="blob-num js-line-number" data-line-number="611"></td>
        <td id="LC611" class="blob-code js-file-line">    <span class="nx">values</span><span class="p">.</span><span class="nx">splice</span><span class="p">(</span><span class="mi">0</span><span class="p">,</span> <span class="nb">Math</span><span class="p">.</span><span class="nx">max</span><span class="p">(</span><span class="mi">0</span><span class="p">,</span> <span class="nb">Math</span><span class="p">.</span><span class="nx">min</span><span class="p">(</span><span class="nx">size</span><span class="p">,</span> <span class="nb">Math</span><span class="p">.</span><span class="nx">round</span><span class="p">((</span><span class="nx">start1</span> <span class="o">-</span> <span class="nx">start</span><span class="p">)</span> <span class="o">/</span> <span class="nx">step</span><span class="p">))));</span></td>
      </tr>
      <tr>
        <td id="L612" class="blob-num js-line-number" data-line-number="612"></td>
        <td id="LC612" class="blob-code js-file-line">    <span class="nx">start</span> <span class="o">=</span> <span class="nx">start1</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L613" class="blob-num js-line-number" data-line-number="613"></td>
        <td id="LC613" class="blob-code js-file-line">    <span class="nx">stop</span> <span class="o">=</span> <span class="nx">stop1</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L614" class="blob-num js-line-number" data-line-number="614"></td>
        <td id="LC614" class="blob-code js-file-line">  <span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L615" class="blob-num js-line-number" data-line-number="615"></td>
        <td id="LC615" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L616" class="blob-num js-line-number" data-line-number="616"></td>
        <td id="LC616" class="blob-code js-file-line">  <span class="c1">//</span></td>
      </tr>
      <tr>
        <td id="L617" class="blob-num js-line-number" data-line-number="617"></td>
        <td id="LC617" class="blob-code js-file-line">  <span class="nx">metric</span><span class="p">.</span><span class="nx">valueAt</span> <span class="o">=</span> <span class="kd">function</span><span class="p">(</span><span class="nx">i</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L618" class="blob-num js-line-number" data-line-number="618"></td>
        <td id="LC618" class="blob-code js-file-line">    <span class="k">return</span> <span class="nx">values</span><span class="p">[</span><span class="nx">i</span><span class="p">];</span></td>
      </tr>
      <tr>
        <td id="L619" class="blob-num js-line-number" data-line-number="619"></td>
        <td id="LC619" class="blob-code js-file-line">  <span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L620" class="blob-num js-line-number" data-line-number="620"></td>
        <td id="LC620" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L621" class="blob-num js-line-number" data-line-number="621"></td>
        <td id="LC621" class="blob-code js-file-line">  <span class="c1">//</span></td>
      </tr>
      <tr>
        <td id="L622" class="blob-num js-line-number" data-line-number="622"></td>
        <td id="LC622" class="blob-code js-file-line">  <span class="nx">metric</span><span class="p">.</span><span class="nx">shift</span> <span class="o">=</span> <span class="kd">function</span><span class="p">(</span><span class="nx">offset</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L623" class="blob-num js-line-number" data-line-number="623"></td>
        <td id="LC623" class="blob-code js-file-line">    <span class="k">return</span> <span class="nx">context</span><span class="p">.</span><span class="nx">metric</span><span class="p">(</span><span class="nx">cubism_metricShift</span><span class="p">(</span><span class="nx">request</span><span class="p">,</span> <span class="o">+</span><span class="nx">offset</span><span class="p">));</span></td>
      </tr>
      <tr>
        <td id="L624" class="blob-num js-line-number" data-line-number="624"></td>
        <td id="LC624" class="blob-code js-file-line">  <span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L625" class="blob-num js-line-number" data-line-number="625"></td>
        <td id="LC625" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L626" class="blob-num js-line-number" data-line-number="626"></td>
        <td id="LC626" class="blob-code js-file-line">  <span class="c1">//</span></td>
      </tr>
      <tr>
        <td id="L627" class="blob-num js-line-number" data-line-number="627"></td>
        <td id="LC627" class="blob-code js-file-line">  <span class="nx">metric</span><span class="p">.</span><span class="nx">on</span> <span class="o">=</span> <span class="kd">function</span><span class="p">(</span><span class="nx">type</span><span class="p">,</span> <span class="nx">listener</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L628" class="blob-num js-line-number" data-line-number="628"></td>
        <td id="LC628" class="blob-code js-file-line">    <span class="k">if</span> <span class="p">(</span><span class="o">!</span><span class="nx">arguments</span><span class="p">.</span><span class="nx">length</span><span class="p">)</span> <span class="k">return</span> <span class="nx">event</span><span class="p">.</span><span class="nx">on</span><span class="p">(</span><span class="nx">type</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L629" class="blob-num js-line-number" data-line-number="629"></td>
        <td id="LC629" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L630" class="blob-num js-line-number" data-line-number="630"></td>
        <td id="LC630" class="blob-code js-file-line">    <span class="c1">// If there are no listeners, then stop listening to the context,</span></td>
      </tr>
      <tr>
        <td id="L631" class="blob-num js-line-number" data-line-number="631"></td>
        <td id="LC631" class="blob-code js-file-line">    <span class="c1">// and avoid unnecessary fetches.</span></td>
      </tr>
      <tr>
        <td id="L632" class="blob-num js-line-number" data-line-number="632"></td>
        <td id="LC632" class="blob-code js-file-line">    <span class="k">if</span> <span class="p">(</span><span class="nx">listener</span> <span class="o">==</span> <span class="kc">null</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L633" class="blob-num js-line-number" data-line-number="633"></td>
        <td id="LC633" class="blob-code js-file-line">      <span class="k">if</span> <span class="p">(</span><span class="nx">event</span><span class="p">.</span><span class="nx">on</span><span class="p">(</span><span class="nx">type</span><span class="p">)</span> <span class="o">!=</span> <span class="kc">null</span> <span class="o">&amp;&amp;</span> <span class="o">--</span><span class="nx">listening</span> <span class="o">==</span> <span class="mi">0</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L634" class="blob-num js-line-number" data-line-number="634"></td>
        <td id="LC634" class="blob-code js-file-line">        <span class="nx">context</span><span class="p">.</span><span class="nx">on</span><span class="p">(</span><span class="s2">&quot;prepare&quot;</span> <span class="o">+</span> <span class="nx">id</span><span class="p">,</span> <span class="kc">null</span><span class="p">).</span><span class="nx">on</span><span class="p">(</span><span class="s2">&quot;beforechange&quot;</span> <span class="o">+</span> <span class="nx">id</span><span class="p">,</span> <span class="kc">null</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L635" class="blob-num js-line-number" data-line-number="635"></td>
        <td id="LC635" class="blob-code js-file-line">      <span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L636" class="blob-num js-line-number" data-line-number="636"></td>
        <td id="LC636" class="blob-code js-file-line">    <span class="p">}</span> <span class="k">else</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L637" class="blob-num js-line-number" data-line-number="637"></td>
        <td id="LC637" class="blob-code js-file-line">      <span class="k">if</span> <span class="p">(</span><span class="nx">event</span><span class="p">.</span><span class="nx">on</span><span class="p">(</span><span class="nx">type</span><span class="p">)</span> <span class="o">==</span> <span class="kc">null</span> <span class="o">&amp;&amp;</span> <span class="o">++</span><span class="nx">listening</span> <span class="o">==</span> <span class="mi">1</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L638" class="blob-num js-line-number" data-line-number="638"></td>
        <td id="LC638" class="blob-code js-file-line">        <span class="nx">context</span><span class="p">.</span><span class="nx">on</span><span class="p">(</span><span class="s2">&quot;prepare&quot;</span> <span class="o">+</span> <span class="nx">id</span><span class="p">,</span> <span class="nx">prepare</span><span class="p">).</span><span class="nx">on</span><span class="p">(</span><span class="s2">&quot;beforechange&quot;</span> <span class="o">+</span> <span class="nx">id</span><span class="p">,</span> <span class="nx">beforechange</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L639" class="blob-num js-line-number" data-line-number="639"></td>
        <td id="LC639" class="blob-code js-file-line">      <span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L640" class="blob-num js-line-number" data-line-number="640"></td>
        <td id="LC640" class="blob-code js-file-line">    <span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L641" class="blob-num js-line-number" data-line-number="641"></td>
        <td id="LC641" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L642" class="blob-num js-line-number" data-line-number="642"></td>
        <td id="LC642" class="blob-code js-file-line">    <span class="nx">event</span><span class="p">.</span><span class="nx">on</span><span class="p">(</span><span class="nx">type</span><span class="p">,</span> <span class="nx">listener</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L643" class="blob-num js-line-number" data-line-number="643"></td>
        <td id="LC643" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L644" class="blob-num js-line-number" data-line-number="644"></td>
        <td id="LC644" class="blob-code js-file-line">    <span class="c1">// Notify the listener of the current start and stop time, as appropriate.</span></td>
      </tr>
      <tr>
        <td id="L645" class="blob-num js-line-number" data-line-number="645"></td>
        <td id="LC645" class="blob-code js-file-line">    <span class="c1">// This way, charts can display synchronous metrics immediately.</span></td>
      </tr>
      <tr>
        <td id="L646" class="blob-num js-line-number" data-line-number="646"></td>
        <td id="LC646" class="blob-code js-file-line">    <span class="k">if</span> <span class="p">(</span><span class="nx">listener</span> <span class="o">!=</span> <span class="kc">null</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L647" class="blob-num js-line-number" data-line-number="647"></td>
        <td id="LC647" class="blob-code js-file-line">      <span class="k">if</span> <span class="p">(</span><span class="sr">/^change(\.|$)/</span><span class="p">.</span><span class="nx">test</span><span class="p">(</span><span class="nx">type</span><span class="p">))</span> <span class="nx">listener</span><span class="p">.</span><span class="nx">call</span><span class="p">(</span><span class="nx">context</span><span class="p">,</span> <span class="nx">start</span><span class="p">,</span> <span class="nx">stop</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L648" class="blob-num js-line-number" data-line-number="648"></td>
        <td id="LC648" class="blob-code js-file-line">    <span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L649" class="blob-num js-line-number" data-line-number="649"></td>
        <td id="LC649" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L650" class="blob-num js-line-number" data-line-number="650"></td>
        <td id="LC650" class="blob-code js-file-line">    <span class="k">return</span> <span class="nx">metric</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L651" class="blob-num js-line-number" data-line-number="651"></td>
        <td id="LC651" class="blob-code js-file-line">  <span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L652" class="blob-num js-line-number" data-line-number="652"></td>
        <td id="LC652" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L653" class="blob-num js-line-number" data-line-number="653"></td>
        <td id="LC653" class="blob-code js-file-line">  <span class="c1">//</span></td>
      </tr>
      <tr>
        <td id="L654" class="blob-num js-line-number" data-line-number="654"></td>
        <td id="LC654" class="blob-code js-file-line">  <span class="k">if</span> <span class="p">(</span><span class="nx">arguments</span><span class="p">.</span><span class="nx">length</span> <span class="o">&gt;</span> <span class="mi">1</span><span class="p">)</span> <span class="nx">metric</span><span class="p">.</span><span class="nx">toString</span> <span class="o">=</span> <span class="kd">function</span><span class="p">()</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L655" class="blob-num js-line-number" data-line-number="655"></td>
        <td id="LC655" class="blob-code js-file-line">    <span class="k">return</span> <span class="nx">name</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L656" class="blob-num js-line-number" data-line-number="656"></td>
        <td id="LC656" class="blob-code js-file-line">  <span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L657" class="blob-num js-line-number" data-line-number="657"></td>
        <td id="LC657" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L658" class="blob-num js-line-number" data-line-number="658"></td>
        <td id="LC658" class="blob-code js-file-line">  <span class="k">return</span> <span class="nx">metric</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L659" class="blob-num js-line-number" data-line-number="659"></td>
        <td id="LC659" class="blob-code js-file-line"><span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L660" class="blob-num js-line-number" data-line-number="660"></td>
        <td id="LC660" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L661" class="blob-num js-line-number" data-line-number="661"></td>
        <td id="LC661" class="blob-code js-file-line"><span class="c1">// Number of metric to refetch each period, in case of lag.</span></td>
      </tr>
      <tr>
        <td id="L662" class="blob-num js-line-number" data-line-number="662"></td>
        <td id="LC662" class="blob-code js-file-line"><span class="kd">var</span> <span class="nx">cubism_metricOverlap</span> <span class="o">=</span> <span class="mi">6</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L663" class="blob-num js-line-number" data-line-number="663"></td>
        <td id="LC663" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L664" class="blob-num js-line-number" data-line-number="664"></td>
        <td id="LC664" class="blob-code js-file-line"><span class="c1">// Wraps the specified request implementation, and shifts time by the given offset.</span></td>
      </tr>
      <tr>
        <td id="L665" class="blob-num js-line-number" data-line-number="665"></td>
        <td id="LC665" class="blob-code js-file-line"><span class="kd">function</span> <span class="nx">cubism_metricShift</span><span class="p">(</span><span class="nx">request</span><span class="p">,</span> <span class="nx">offset</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L666" class="blob-num js-line-number" data-line-number="666"></td>
        <td id="LC666" class="blob-code js-file-line">  <span class="k">return</span> <span class="kd">function</span><span class="p">(</span><span class="nx">start</span><span class="p">,</span> <span class="nx">stop</span><span class="p">,</span> <span class="nx">step</span><span class="p">,</span> <span class="nx">callback</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L667" class="blob-num js-line-number" data-line-number="667"></td>
        <td id="LC667" class="blob-code js-file-line">    <span class="nx">request</span><span class="p">(</span><span class="k">new</span> <span class="nb">Date</span><span class="p">(</span><span class="o">+</span><span class="nx">start</span> <span class="o">+</span> <span class="nx">offset</span><span class="p">),</span> <span class="k">new</span> <span class="nb">Date</span><span class="p">(</span><span class="o">+</span><span class="nx">stop</span> <span class="o">+</span> <span class="nx">offset</span><span class="p">),</span> <span class="nx">step</span><span class="p">,</span> <span class="nx">callback</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L668" class="blob-num js-line-number" data-line-number="668"></td>
        <td id="LC668" class="blob-code js-file-line">  <span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L669" class="blob-num js-line-number" data-line-number="669"></td>
        <td id="LC669" class="blob-code js-file-line"><span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L670" class="blob-num js-line-number" data-line-number="670"></td>
        <td id="LC670" class="blob-code js-file-line"><span class="kd">function</span> <span class="nx">cubism_metricConstant</span><span class="p">(</span><span class="nx">context</span><span class="p">,</span> <span class="nx">value</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L671" class="blob-num js-line-number" data-line-number="671"></td>
        <td id="LC671" class="blob-code js-file-line">  <span class="nx">cubism_metric</span><span class="p">.</span><span class="nx">call</span><span class="p">(</span><span class="k">this</span><span class="p">,</span> <span class="nx">context</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L672" class="blob-num js-line-number" data-line-number="672"></td>
        <td id="LC672" class="blob-code js-file-line">  <span class="nx">value</span> <span class="o">=</span> <span class="o">+</span><span class="nx">value</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L673" class="blob-num js-line-number" data-line-number="673"></td>
        <td id="LC673" class="blob-code js-file-line">  <span class="kd">var</span> <span class="nx">name</span> <span class="o">=</span> <span class="nx">value</span> <span class="o">+</span> <span class="s2">&quot;&quot;</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L674" class="blob-num js-line-number" data-line-number="674"></td>
        <td id="LC674" class="blob-code js-file-line">  <span class="k">this</span><span class="p">.</span><span class="nx">valueOf</span> <span class="o">=</span> <span class="kd">function</span><span class="p">()</span> <span class="p">{</span> <span class="k">return</span> <span class="nx">value</span><span class="p">;</span> <span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L675" class="blob-num js-line-number" data-line-number="675"></td>
        <td id="LC675" class="blob-code js-file-line">  <span class="k">this</span><span class="p">.</span><span class="nx">toString</span> <span class="o">=</span> <span class="kd">function</span><span class="p">()</span> <span class="p">{</span> <span class="k">return</span> <span class="nx">name</span><span class="p">;</span> <span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L676" class="blob-num js-line-number" data-line-number="676"></td>
        <td id="LC676" class="blob-code js-file-line"><span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L677" class="blob-num js-line-number" data-line-number="677"></td>
        <td id="LC677" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L678" class="blob-num js-line-number" data-line-number="678"></td>
        <td id="LC678" class="blob-code js-file-line"><span class="kd">var</span> <span class="nx">cubism_metricConstantPrototype</span> <span class="o">=</span> <span class="nx">cubism_metricConstant</span><span class="p">.</span><span class="nx">prototype</span> <span class="o">=</span> <span class="nb">Object</span><span class="p">.</span><span class="nx">create</span><span class="p">(</span><span class="nx">cubism_metric</span><span class="p">.</span><span class="nx">prototype</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L679" class="blob-num js-line-number" data-line-number="679"></td>
        <td id="LC679" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L680" class="blob-num js-line-number" data-line-number="680"></td>
        <td id="LC680" class="blob-code js-file-line"><span class="nx">cubism_metricConstantPrototype</span><span class="p">.</span><span class="nx">valueAt</span> <span class="o">=</span> <span class="kd">function</span><span class="p">()</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L681" class="blob-num js-line-number" data-line-number="681"></td>
        <td id="LC681" class="blob-code js-file-line">  <span class="k">return</span> <span class="o">+</span><span class="k">this</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L682" class="blob-num js-line-number" data-line-number="682"></td>
        <td id="LC682" class="blob-code js-file-line"><span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L683" class="blob-num js-line-number" data-line-number="683"></td>
        <td id="LC683" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L684" class="blob-num js-line-number" data-line-number="684"></td>
        <td id="LC684" class="blob-code js-file-line"><span class="nx">cubism_metricConstantPrototype</span><span class="p">.</span><span class="nx">extent</span> <span class="o">=</span> <span class="kd">function</span><span class="p">()</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L685" class="blob-num js-line-number" data-line-number="685"></td>
        <td id="LC685" class="blob-code js-file-line">  <span class="k">return</span> <span class="p">[</span><span class="o">+</span><span class="k">this</span><span class="p">,</span> <span class="o">+</span><span class="k">this</span><span class="p">];</span></td>
      </tr>
      <tr>
        <td id="L686" class="blob-num js-line-number" data-line-number="686"></td>
        <td id="LC686" class="blob-code js-file-line"><span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L687" class="blob-num js-line-number" data-line-number="687"></td>
        <td id="LC687" class="blob-code js-file-line"><span class="kd">function</span> <span class="nx">cubism_metricOperator</span><span class="p">(</span><span class="nx">name</span><span class="p">,</span> <span class="nx">operate</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L688" class="blob-num js-line-number" data-line-number="688"></td>
        <td id="LC688" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L689" class="blob-num js-line-number" data-line-number="689"></td>
        <td id="LC689" class="blob-code js-file-line">  <span class="kd">function</span> <span class="nx">cubism_metricOperator</span><span class="p">(</span><span class="nx">left</span><span class="p">,</span> <span class="nx">right</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L690" class="blob-num js-line-number" data-line-number="690"></td>
        <td id="LC690" class="blob-code js-file-line">    <span class="k">if</span> <span class="p">(</span><span class="o">!</span><span class="p">(</span><span class="nx">right</span> <span class="k">instanceof</span> <span class="nx">cubism_metric</span><span class="p">))</span> <span class="nx">right</span> <span class="o">=</span> <span class="k">new</span> <span class="nx">cubism_metricConstant</span><span class="p">(</span><span class="nx">left</span><span class="p">.</span><span class="nx">context</span><span class="p">,</span> <span class="nx">right</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L691" class="blob-num js-line-number" data-line-number="691"></td>
        <td id="LC691" class="blob-code js-file-line">    <span class="k">else</span> <span class="k">if</span> <span class="p">(</span><span class="nx">left</span><span class="p">.</span><span class="nx">context</span> <span class="o">!==</span> <span class="nx">right</span><span class="p">.</span><span class="nx">context</span><span class="p">)</span> <span class="k">throw</span> <span class="k">new</span> <span class="nb">Error</span><span class="p">(</span><span class="s2">&quot;mismatch context&quot;</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L692" class="blob-num js-line-number" data-line-number="692"></td>
        <td id="LC692" class="blob-code js-file-line">    <span class="nx">cubism_metric</span><span class="p">.</span><span class="nx">call</span><span class="p">(</span><span class="k">this</span><span class="p">,</span> <span class="nx">left</span><span class="p">.</span><span class="nx">context</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L693" class="blob-num js-line-number" data-line-number="693"></td>
        <td id="LC693" class="blob-code js-file-line">    <span class="k">this</span><span class="p">.</span><span class="nx">left</span> <span class="o">=</span> <span class="nx">left</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L694" class="blob-num js-line-number" data-line-number="694"></td>
        <td id="LC694" class="blob-code js-file-line">    <span class="k">this</span><span class="p">.</span><span class="nx">right</span> <span class="o">=</span> <span class="nx">right</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L695" class="blob-num js-line-number" data-line-number="695"></td>
        <td id="LC695" class="blob-code js-file-line">    <span class="k">this</span><span class="p">.</span><span class="nx">toString</span> <span class="o">=</span> <span class="kd">function</span><span class="p">()</span> <span class="p">{</span> <span class="k">return</span> <span class="nx">left</span> <span class="o">+</span> <span class="s2">&quot; &quot;</span> <span class="o">+</span> <span class="nx">name</span> <span class="o">+</span> <span class="s2">&quot; &quot;</span> <span class="o">+</span> <span class="nx">right</span><span class="p">;</span> <span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L696" class="blob-num js-line-number" data-line-number="696"></td>
        <td id="LC696" class="blob-code js-file-line">  <span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L697" class="blob-num js-line-number" data-line-number="697"></td>
        <td id="LC697" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L698" class="blob-num js-line-number" data-line-number="698"></td>
        <td id="LC698" class="blob-code js-file-line">  <span class="kd">var</span> <span class="nx">cubism_metricOperatorPrototype</span> <span class="o">=</span> <span class="nx">cubism_metricOperator</span><span class="p">.</span><span class="nx">prototype</span> <span class="o">=</span> <span class="nb">Object</span><span class="p">.</span><span class="nx">create</span><span class="p">(</span><span class="nx">cubism_metric</span><span class="p">.</span><span class="nx">prototype</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L699" class="blob-num js-line-number" data-line-number="699"></td>
        <td id="LC699" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L700" class="blob-num js-line-number" data-line-number="700"></td>
        <td id="LC700" class="blob-code js-file-line">  <span class="nx">cubism_metricOperatorPrototype</span><span class="p">.</span><span class="nx">valueAt</span> <span class="o">=</span> <span class="kd">function</span><span class="p">(</span><span class="nx">i</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L701" class="blob-num js-line-number" data-line-number="701"></td>
        <td id="LC701" class="blob-code js-file-line">    <span class="k">return</span> <span class="nx">operate</span><span class="p">(</span><span class="k">this</span><span class="p">.</span><span class="nx">left</span><span class="p">.</span><span class="nx">valueAt</span><span class="p">(</span><span class="nx">i</span><span class="p">),</span> <span class="k">this</span><span class="p">.</span><span class="nx">right</span><span class="p">.</span><span class="nx">valueAt</span><span class="p">(</span><span class="nx">i</span><span class="p">));</span></td>
      </tr>
      <tr>
        <td id="L702" class="blob-num js-line-number" data-line-number="702"></td>
        <td id="LC702" class="blob-code js-file-line">  <span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L703" class="blob-num js-line-number" data-line-number="703"></td>
        <td id="LC703" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L704" class="blob-num js-line-number" data-line-number="704"></td>
        <td id="LC704" class="blob-code js-file-line">  <span class="nx">cubism_metricOperatorPrototype</span><span class="p">.</span><span class="nx">shift</span> <span class="o">=</span> <span class="kd">function</span><span class="p">(</span><span class="nx">offset</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L705" class="blob-num js-line-number" data-line-number="705"></td>
        <td id="LC705" class="blob-code js-file-line">    <span class="k">return</span> <span class="k">new</span> <span class="nx">cubism_metricOperator</span><span class="p">(</span><span class="k">this</span><span class="p">.</span><span class="nx">left</span><span class="p">.</span><span class="nx">shift</span><span class="p">(</span><span class="nx">offset</span><span class="p">),</span> <span class="k">this</span><span class="p">.</span><span class="nx">right</span><span class="p">.</span><span class="nx">shift</span><span class="p">(</span><span class="nx">offset</span><span class="p">));</span></td>
      </tr>
      <tr>
        <td id="L706" class="blob-num js-line-number" data-line-number="706"></td>
        <td id="LC706" class="blob-code js-file-line">  <span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L707" class="blob-num js-line-number" data-line-number="707"></td>
        <td id="LC707" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L708" class="blob-num js-line-number" data-line-number="708"></td>
        <td id="LC708" class="blob-code js-file-line">  <span class="nx">cubism_metricOperatorPrototype</span><span class="p">.</span><span class="nx">on</span> <span class="o">=</span> <span class="kd">function</span><span class="p">(</span><span class="nx">type</span><span class="p">,</span> <span class="nx">listener</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L709" class="blob-num js-line-number" data-line-number="709"></td>
        <td id="LC709" class="blob-code js-file-line">    <span class="k">if</span> <span class="p">(</span><span class="nx">arguments</span><span class="p">.</span><span class="nx">length</span> <span class="o">&lt;</span> <span class="mi">2</span><span class="p">)</span> <span class="k">return</span> <span class="k">this</span><span class="p">.</span><span class="nx">left</span><span class="p">.</span><span class="nx">on</span><span class="p">(</span><span class="nx">type</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L710" class="blob-num js-line-number" data-line-number="710"></td>
        <td id="LC710" class="blob-code js-file-line">    <span class="k">this</span><span class="p">.</span><span class="nx">left</span><span class="p">.</span><span class="nx">on</span><span class="p">(</span><span class="nx">type</span><span class="p">,</span> <span class="nx">listener</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L711" class="blob-num js-line-number" data-line-number="711"></td>
        <td id="LC711" class="blob-code js-file-line">    <span class="k">this</span><span class="p">.</span><span class="nx">right</span><span class="p">.</span><span class="nx">on</span><span class="p">(</span><span class="nx">type</span><span class="p">,</span> <span class="nx">listener</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L712" class="blob-num js-line-number" data-line-number="712"></td>
        <td id="LC712" class="blob-code js-file-line">    <span class="k">return</span> <span class="k">this</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L713" class="blob-num js-line-number" data-line-number="713"></td>
        <td id="LC713" class="blob-code js-file-line">  <span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L714" class="blob-num js-line-number" data-line-number="714"></td>
        <td id="LC714" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L715" class="blob-num js-line-number" data-line-number="715"></td>
        <td id="LC715" class="blob-code js-file-line">  <span class="k">return</span> <span class="kd">function</span><span class="p">(</span><span class="nx">right</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L716" class="blob-num js-line-number" data-line-number="716"></td>
        <td id="LC716" class="blob-code js-file-line">    <span class="k">return</span> <span class="k">new</span> <span class="nx">cubism_metricOperator</span><span class="p">(</span><span class="k">this</span><span class="p">,</span> <span class="nx">right</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L717" class="blob-num js-line-number" data-line-number="717"></td>
        <td id="LC717" class="blob-code js-file-line">  <span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L718" class="blob-num js-line-number" data-line-number="718"></td>
        <td id="LC718" class="blob-code js-file-line"><span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L719" class="blob-num js-line-number" data-line-number="719"></td>
        <td id="LC719" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L720" class="blob-num js-line-number" data-line-number="720"></td>
        <td id="LC720" class="blob-code js-file-line"><span class="nx">cubism_metricPrototype</span><span class="p">.</span><span class="nx">add</span> <span class="o">=</span> <span class="nx">cubism_metricOperator</span><span class="p">(</span><span class="s2">&quot;+&quot;</span><span class="p">,</span> <span class="kd">function</span><span class="p">(</span><span class="nx">left</span><span class="p">,</span> <span class="nx">right</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L721" class="blob-num js-line-number" data-line-number="721"></td>
        <td id="LC721" class="blob-code js-file-line">  <span class="k">return</span> <span class="nx">left</span> <span class="o">+</span> <span class="nx">right</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L722" class="blob-num js-line-number" data-line-number="722"></td>
        <td id="LC722" class="blob-code js-file-line"><span class="p">});</span></td>
      </tr>
      <tr>
        <td id="L723" class="blob-num js-line-number" data-line-number="723"></td>
        <td id="LC723" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L724" class="blob-num js-line-number" data-line-number="724"></td>
        <td id="LC724" class="blob-code js-file-line"><span class="nx">cubism_metricPrototype</span><span class="p">.</span><span class="nx">subtract</span> <span class="o">=</span> <span class="nx">cubism_metricOperator</span><span class="p">(</span><span class="s2">&quot;-&quot;</span><span class="p">,</span> <span class="kd">function</span><span class="p">(</span><span class="nx">left</span><span class="p">,</span> <span class="nx">right</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L725" class="blob-num js-line-number" data-line-number="725"></td>
        <td id="LC725" class="blob-code js-file-line">  <span class="k">return</span> <span class="nx">left</span> <span class="o">-</span> <span class="nx">right</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L726" class="blob-num js-line-number" data-line-number="726"></td>
        <td id="LC726" class="blob-code js-file-line"><span class="p">});</span></td>
      </tr>
      <tr>
        <td id="L727" class="blob-num js-line-number" data-line-number="727"></td>
        <td id="LC727" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L728" class="blob-num js-line-number" data-line-number="728"></td>
        <td id="LC728" class="blob-code js-file-line"><span class="nx">cubism_metricPrototype</span><span class="p">.</span><span class="nx">multiply</span> <span class="o">=</span> <span class="nx">cubism_metricOperator</span><span class="p">(</span><span class="s2">&quot;*&quot;</span><span class="p">,</span> <span class="kd">function</span><span class="p">(</span><span class="nx">left</span><span class="p">,</span> <span class="nx">right</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L729" class="blob-num js-line-number" data-line-number="729"></td>
        <td id="LC729" class="blob-code js-file-line">  <span class="k">return</span> <span class="nx">left</span> <span class="o">*</span> <span class="nx">right</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L730" class="blob-num js-line-number" data-line-number="730"></td>
        <td id="LC730" class="blob-code js-file-line"><span class="p">});</span></td>
      </tr>
      <tr>
        <td id="L731" class="blob-num js-line-number" data-line-number="731"></td>
        <td id="LC731" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L732" class="blob-num js-line-number" data-line-number="732"></td>
        <td id="LC732" class="blob-code js-file-line"><span class="nx">cubism_metricPrototype</span><span class="p">.</span><span class="nx">divide</span> <span class="o">=</span> <span class="nx">cubism_metricOperator</span><span class="p">(</span><span class="s2">&quot;/&quot;</span><span class="p">,</span> <span class="kd">function</span><span class="p">(</span><span class="nx">left</span><span class="p">,</span> <span class="nx">right</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L733" class="blob-num js-line-number" data-line-number="733"></td>
        <td id="LC733" class="blob-code js-file-line">  <span class="k">return</span> <span class="nx">left</span> <span class="o">/</span> <span class="nx">right</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L734" class="blob-num js-line-number" data-line-number="734"></td>
        <td id="LC734" class="blob-code js-file-line"><span class="p">});</span></td>
      </tr>
      <tr>
        <td id="L735" class="blob-num js-line-number" data-line-number="735"></td>
        <td id="LC735" class="blob-code js-file-line"><span class="nx">cubism_contextPrototype</span><span class="p">.</span><span class="nx">horizon</span> <span class="o">=</span> <span class="kd">function</span><span class="p">()</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L736" class="blob-num js-line-number" data-line-number="736"></td>
        <td id="LC736" class="blob-code js-file-line">  <span class="kd">var</span> <span class="nx">context</span> <span class="o">=</span> <span class="k">this</span><span class="p">,</span></td>
      </tr>
      <tr>
        <td id="L737" class="blob-num js-line-number" data-line-number="737"></td>
        <td id="LC737" class="blob-code js-file-line">      <span class="nx">mode</span> <span class="o">=</span> <span class="s2">&quot;offset&quot;</span><span class="p">,</span></td>
      </tr>
      <tr>
        <td id="L738" class="blob-num js-line-number" data-line-number="738"></td>
        <td id="LC738" class="blob-code js-file-line">      <span class="nx">buffer</span> <span class="o">=</span> <span class="nb">document</span><span class="p">.</span><span class="nx">createElement</span><span class="p">(</span><span class="s2">&quot;canvas&quot;</span><span class="p">),</span></td>
      </tr>
      <tr>
        <td id="L739" class="blob-num js-line-number" data-line-number="739"></td>
        <td id="LC739" class="blob-code js-file-line">      <span class="nx">width</span> <span class="o">=</span> <span class="nx">buffer</span><span class="p">.</span><span class="nx">width</span> <span class="o">=</span> <span class="nx">context</span><span class="p">.</span><span class="nx">size</span><span class="p">(),</span></td>
      </tr>
      <tr>
        <td id="L740" class="blob-num js-line-number" data-line-number="740"></td>
        <td id="LC740" class="blob-code js-file-line">      <span class="nx">height</span> <span class="o">=</span> <span class="nx">buffer</span><span class="p">.</span><span class="nx">height</span> <span class="o">=</span> <span class="mi">30</span><span class="p">,</span></td>
      </tr>
      <tr>
        <td id="L741" class="blob-num js-line-number" data-line-number="741"></td>
        <td id="LC741" class="blob-code js-file-line">      <span class="nx">scale</span> <span class="o">=</span> <span class="nx">d3</span><span class="p">.</span><span class="nx">scale</span><span class="p">.</span><span class="nx">linear</span><span class="p">().</span><span class="nx">interpolate</span><span class="p">(</span><span class="nx">d3</span><span class="p">.</span><span class="nx">interpolateRound</span><span class="p">),</span></td>
      </tr>
      <tr>
        <td id="L742" class="blob-num js-line-number" data-line-number="742"></td>
        <td id="LC742" class="blob-code js-file-line">      <span class="nx">metric</span> <span class="o">=</span> <span class="nx">cubism_identity</span><span class="p">,</span></td>
      </tr>
      <tr>
        <td id="L743" class="blob-num js-line-number" data-line-number="743"></td>
        <td id="LC743" class="blob-code js-file-line">      <span class="nx">extent</span> <span class="o">=</span> <span class="kc">null</span><span class="p">,</span></td>
      </tr>
      <tr>
        <td id="L744" class="blob-num js-line-number" data-line-number="744"></td>
        <td id="LC744" class="blob-code js-file-line">      <span class="nx">title</span> <span class="o">=</span> <span class="nx">cubism_identity</span><span class="p">,</span></td>
      </tr>
      <tr>
        <td id="L745" class="blob-num js-line-number" data-line-number="745"></td>
        <td id="LC745" class="blob-code js-file-line">      <span class="nx">format</span> <span class="o">=</span> <span class="nx">d3</span><span class="p">.</span><span class="nx">format</span><span class="p">(</span><span class="s2">&quot;.2s&quot;</span><span class="p">),</span></td>
      </tr>
      <tr>
        <td id="L746" class="blob-num js-line-number" data-line-number="746"></td>
        <td id="LC746" class="blob-code js-file-line">      <span class="nx">colors</span> <span class="o">=</span> <span class="p">[</span><span class="s2">&quot;#08519c&quot;</span><span class="p">,</span><span class="s2">&quot;#3182bd&quot;</span><span class="p">,</span><span class="s2">&quot;#6baed6&quot;</span><span class="p">,</span><span class="s2">&quot;#bdd7e7&quot;</span><span class="p">,</span><span class="s2">&quot;#bae4b3&quot;</span><span class="p">,</span><span class="s2">&quot;#74c476&quot;</span><span class="p">,</span><span class="s2">&quot;#31a354&quot;</span><span class="p">,</span><span class="s2">&quot;#006d2c&quot;</span><span class="p">];</span></td>
      </tr>
      <tr>
        <td id="L747" class="blob-num js-line-number" data-line-number="747"></td>
        <td id="LC747" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L748" class="blob-num js-line-number" data-line-number="748"></td>
        <td id="LC748" class="blob-code js-file-line">  <span class="kd">function</span> <span class="nx">horizon</span><span class="p">(</span><span class="nx">selection</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L749" class="blob-num js-line-number" data-line-number="749"></td>
        <td id="LC749" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L750" class="blob-num js-line-number" data-line-number="750"></td>
        <td id="LC750" class="blob-code js-file-line">    <span class="nx">selection</span></td>
      </tr>
      <tr>
        <td id="L751" class="blob-num js-line-number" data-line-number="751"></td>
        <td id="LC751" class="blob-code js-file-line">        <span class="p">.</span><span class="nx">on</span><span class="p">(</span><span class="s2">&quot;mousemove.horizon&quot;</span><span class="p">,</span> <span class="kd">function</span><span class="p">()</span> <span class="p">{</span> <span class="nx">context</span><span class="p">.</span><span class="nx">focus</span><span class="p">(</span><span class="nb">Math</span><span class="p">.</span><span class="nx">round</span><span class="p">(</span><span class="nx">d3</span><span class="p">.</span><span class="nx">mouse</span><span class="p">(</span><span class="k">this</span><span class="p">)[</span><span class="mi">0</span><span class="p">]));</span> <span class="p">})</span></td>
      </tr>
      <tr>
        <td id="L752" class="blob-num js-line-number" data-line-number="752"></td>
        <td id="LC752" class="blob-code js-file-line">        <span class="p">.</span><span class="nx">on</span><span class="p">(</span><span class="s2">&quot;mouseout.horizon&quot;</span><span class="p">,</span> <span class="kd">function</span><span class="p">()</span> <span class="p">{</span> <span class="nx">context</span><span class="p">.</span><span class="nx">focus</span><span class="p">(</span><span class="kc">null</span><span class="p">);</span> <span class="p">});</span></td>
      </tr>
      <tr>
        <td id="L753" class="blob-num js-line-number" data-line-number="753"></td>
        <td id="LC753" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L754" class="blob-num js-line-number" data-line-number="754"></td>
        <td id="LC754" class="blob-code js-file-line">    <span class="nx">selection</span><span class="p">.</span><span class="nx">append</span><span class="p">(</span><span class="s2">&quot;canvas&quot;</span><span class="p">)</span></td>
      </tr>
      <tr>
        <td id="L755" class="blob-num js-line-number" data-line-number="755"></td>
        <td id="LC755" class="blob-code js-file-line">        <span class="p">.</span><span class="nx">attr</span><span class="p">(</span><span class="s2">&quot;width&quot;</span><span class="p">,</span> <span class="nx">width</span><span class="p">)</span></td>
      </tr>
      <tr>
        <td id="L756" class="blob-num js-line-number" data-line-number="756"></td>
        <td id="LC756" class="blob-code js-file-line">        <span class="p">.</span><span class="nx">attr</span><span class="p">(</span><span class="s2">&quot;height&quot;</span><span class="p">,</span> <span class="nx">height</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L757" class="blob-num js-line-number" data-line-number="757"></td>
        <td id="LC757" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L758" class="blob-num js-line-number" data-line-number="758"></td>
        <td id="LC758" class="blob-code js-file-line">    <span class="nx">selection</span><span class="p">.</span><span class="nx">append</span><span class="p">(</span><span class="s2">&quot;span&quot;</span><span class="p">)</span></td>
      </tr>
      <tr>
        <td id="L759" class="blob-num js-line-number" data-line-number="759"></td>
        <td id="LC759" class="blob-code js-file-line">        <span class="p">.</span><span class="nx">attr</span><span class="p">(</span><span class="s2">&quot;class&quot;</span><span class="p">,</span> <span class="s2">&quot;title&quot;</span><span class="p">)</span></td>
      </tr>
      <tr>
        <td id="L760" class="blob-num js-line-number" data-line-number="760"></td>
        <td id="LC760" class="blob-code js-file-line">        <span class="p">.</span><span class="nx">text</span><span class="p">(</span><span class="nx">title</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L761" class="blob-num js-line-number" data-line-number="761"></td>
        <td id="LC761" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L762" class="blob-num js-line-number" data-line-number="762"></td>
        <td id="LC762" class="blob-code js-file-line">    <span class="nx">selection</span><span class="p">.</span><span class="nx">append</span><span class="p">(</span><span class="s2">&quot;span&quot;</span><span class="p">)</span></td>
      </tr>
      <tr>
        <td id="L763" class="blob-num js-line-number" data-line-number="763"></td>
        <td id="LC763" class="blob-code js-file-line">        <span class="p">.</span><span class="nx">attr</span><span class="p">(</span><span class="s2">&quot;class&quot;</span><span class="p">,</span> <span class="s2">&quot;value&quot;</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L764" class="blob-num js-line-number" data-line-number="764"></td>
        <td id="LC764" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L765" class="blob-num js-line-number" data-line-number="765"></td>
        <td id="LC765" class="blob-code js-file-line">    <span class="nx">selection</span><span class="p">.</span><span class="nx">each</span><span class="p">(</span><span class="kd">function</span><span class="p">(</span><span class="nx">d</span><span class="p">,</span> <span class="nx">i</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L766" class="blob-num js-line-number" data-line-number="766"></td>
        <td id="LC766" class="blob-code js-file-line">      <span class="kd">var</span> <span class="nx">that</span> <span class="o">=</span> <span class="k">this</span><span class="p">,</span></td>
      </tr>
      <tr>
        <td id="L767" class="blob-num js-line-number" data-line-number="767"></td>
        <td id="LC767" class="blob-code js-file-line">          <span class="nx">id</span> <span class="o">=</span> <span class="o">++</span><span class="nx">cubism_id</span><span class="p">,</span></td>
      </tr>
      <tr>
        <td id="L768" class="blob-num js-line-number" data-line-number="768"></td>
        <td id="LC768" class="blob-code js-file-line">          <span class="nx">metric_</span> <span class="o">=</span> <span class="k">typeof</span> <span class="nx">metric</span> <span class="o">===</span> <span class="s2">&quot;function&quot;</span> <span class="o">?</span> <span class="nx">metric</span><span class="p">.</span><span class="nx">call</span><span class="p">(</span><span class="nx">that</span><span class="p">,</span> <span class="nx">d</span><span class="p">,</span> <span class="nx">i</span><span class="p">)</span> <span class="o">:</span> <span class="nx">metric</span><span class="p">,</span></td>
      </tr>
      <tr>
        <td id="L769" class="blob-num js-line-number" data-line-number="769"></td>
        <td id="LC769" class="blob-code js-file-line">          <span class="nx">colors_</span> <span class="o">=</span> <span class="k">typeof</span> <span class="nx">colors</span> <span class="o">===</span> <span class="s2">&quot;function&quot;</span> <span class="o">?</span> <span class="nx">colors</span><span class="p">.</span><span class="nx">call</span><span class="p">(</span><span class="nx">that</span><span class="p">,</span> <span class="nx">d</span><span class="p">,</span> <span class="nx">i</span><span class="p">)</span> <span class="o">:</span> <span class="nx">colors</span><span class="p">,</span></td>
      </tr>
      <tr>
        <td id="L770" class="blob-num js-line-number" data-line-number="770"></td>
        <td id="LC770" class="blob-code js-file-line">          <span class="nx">extent_</span> <span class="o">=</span> <span class="k">typeof</span> <span class="nx">extent</span> <span class="o">===</span> <span class="s2">&quot;function&quot;</span> <span class="o">?</span> <span class="nx">extent</span><span class="p">.</span><span class="nx">call</span><span class="p">(</span><span class="nx">that</span><span class="p">,</span> <span class="nx">d</span><span class="p">,</span> <span class="nx">i</span><span class="p">)</span> <span class="o">:</span> <span class="nx">extent</span><span class="p">,</span></td>
      </tr>
      <tr>
        <td id="L771" class="blob-num js-line-number" data-line-number="771"></td>
        <td id="LC771" class="blob-code js-file-line">          <span class="nx">start</span> <span class="o">=</span> <span class="o">-</span><span class="kc">Infinity</span><span class="p">,</span></td>
      </tr>
      <tr>
        <td id="L772" class="blob-num js-line-number" data-line-number="772"></td>
        <td id="LC772" class="blob-code js-file-line">          <span class="nx">step</span> <span class="o">=</span> <span class="nx">context</span><span class="p">.</span><span class="nx">step</span><span class="p">(),</span></td>
      </tr>
      <tr>
        <td id="L773" class="blob-num js-line-number" data-line-number="773"></td>
        <td id="LC773" class="blob-code js-file-line">          <span class="nx">canvas</span> <span class="o">=</span> <span class="nx">d3</span><span class="p">.</span><span class="nx">select</span><span class="p">(</span><span class="nx">that</span><span class="p">).</span><span class="nx">select</span><span class="p">(</span><span class="s2">&quot;canvas&quot;</span><span class="p">),</span></td>
      </tr>
      <tr>
        <td id="L774" class="blob-num js-line-number" data-line-number="774"></td>
        <td id="LC774" class="blob-code js-file-line">          <span class="nx">span</span> <span class="o">=</span> <span class="nx">d3</span><span class="p">.</span><span class="nx">select</span><span class="p">(</span><span class="nx">that</span><span class="p">).</span><span class="nx">select</span><span class="p">(</span><span class="s2">&quot;.value&quot;</span><span class="p">),</span></td>
      </tr>
      <tr>
        <td id="L775" class="blob-num js-line-number" data-line-number="775"></td>
        <td id="LC775" class="blob-code js-file-line">          <span class="nx">max_</span><span class="p">,</span></td>
      </tr>
      <tr>
        <td id="L776" class="blob-num js-line-number" data-line-number="776"></td>
        <td id="LC776" class="blob-code js-file-line">          <span class="nx">m</span> <span class="o">=</span> <span class="nx">colors_</span><span class="p">.</span><span class="nx">length</span> <span class="o">&gt;&gt;</span> <span class="mi">1</span><span class="p">,</span></td>
      </tr>
      <tr>
        <td id="L777" class="blob-num js-line-number" data-line-number="777"></td>
        <td id="LC777" class="blob-code js-file-line">          <span class="nx">ready</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L778" class="blob-num js-line-number" data-line-number="778"></td>
        <td id="LC778" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L779" class="blob-num js-line-number" data-line-number="779"></td>
        <td id="LC779" class="blob-code js-file-line">      <span class="nx">canvas</span><span class="p">.</span><span class="nx">datum</span><span class="p">({</span><span class="nx">id</span><span class="o">:</span> <span class="nx">id</span><span class="p">,</span> <span class="nx">metric</span><span class="o">:</span> <span class="nx">metric_</span><span class="p">});</span></td>
      </tr>
      <tr>
        <td id="L780" class="blob-num js-line-number" data-line-number="780"></td>
        <td id="LC780" class="blob-code js-file-line">      <span class="nx">canvas</span> <span class="o">=</span> <span class="nx">canvas</span><span class="p">.</span><span class="nx">node</span><span class="p">().</span><span class="nx">getContext</span><span class="p">(</span><span class="s2">&quot;2d&quot;</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L781" class="blob-num js-line-number" data-line-number="781"></td>
        <td id="LC781" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L782" class="blob-num js-line-number" data-line-number="782"></td>
        <td id="LC782" class="blob-code js-file-line">      <span class="kd">function</span> <span class="nx">change</span><span class="p">(</span><span class="nx">start1</span><span class="p">,</span> <span class="nx">stop</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L783" class="blob-num js-line-number" data-line-number="783"></td>
        <td id="LC783" class="blob-code js-file-line">        <span class="nx">canvas</span><span class="p">.</span><span class="nx">save</span><span class="p">();</span></td>
      </tr>
      <tr>
        <td id="L784" class="blob-num js-line-number" data-line-number="784"></td>
        <td id="LC784" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L785" class="blob-num js-line-number" data-line-number="785"></td>
        <td id="LC785" class="blob-code js-file-line">        <span class="c1">// compute the new extent and ready flag</span></td>
      </tr>
      <tr>
        <td id="L786" class="blob-num js-line-number" data-line-number="786"></td>
        <td id="LC786" class="blob-code js-file-line">        <span class="kd">var</span> <span class="nx">extent</span> <span class="o">=</span> <span class="nx">metric_</span><span class="p">.</span><span class="nx">extent</span><span class="p">();</span></td>
      </tr>
      <tr>
        <td id="L787" class="blob-num js-line-number" data-line-number="787"></td>
        <td id="LC787" class="blob-code js-file-line">        <span class="nx">ready</span> <span class="o">=</span> <span class="nx">extent</span><span class="p">.</span><span class="nx">every</span><span class="p">(</span><span class="nb">isFinite</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L788" class="blob-num js-line-number" data-line-number="788"></td>
        <td id="LC788" class="blob-code js-file-line">        <span class="k">if</span> <span class="p">(</span><span class="nx">extent_</span> <span class="o">!=</span> <span class="kc">null</span><span class="p">)</span> <span class="nx">extent</span> <span class="o">=</span> <span class="nx">extent_</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L789" class="blob-num js-line-number" data-line-number="789"></td>
        <td id="LC789" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L790" class="blob-num js-line-number" data-line-number="790"></td>
        <td id="LC790" class="blob-code js-file-line">        <span class="c1">// if this is an update (with no extent change), copy old values!</span></td>
      </tr>
      <tr>
        <td id="L791" class="blob-num js-line-number" data-line-number="791"></td>
        <td id="LC791" class="blob-code js-file-line">        <span class="kd">var</span> <span class="nx">i0</span> <span class="o">=</span> <span class="mi">0</span><span class="p">,</span> <span class="nx">max</span> <span class="o">=</span> <span class="nb">Math</span><span class="p">.</span><span class="nx">max</span><span class="p">(</span><span class="o">-</span><span class="nx">extent</span><span class="p">[</span><span class="mi">0</span><span class="p">],</span> <span class="nx">extent</span><span class="p">[</span><span class="mi">1</span><span class="p">]);</span></td>
      </tr>
      <tr>
        <td id="L792" class="blob-num js-line-number" data-line-number="792"></td>
        <td id="LC792" class="blob-code js-file-line">        <span class="k">if</span> <span class="p">(</span><span class="k">this</span> <span class="o">===</span> <span class="nx">context</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L793" class="blob-num js-line-number" data-line-number="793"></td>
        <td id="LC793" class="blob-code js-file-line">          <span class="k">if</span> <span class="p">(</span><span class="nx">max</span> <span class="o">==</span> <span class="nx">max_</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L794" class="blob-num js-line-number" data-line-number="794"></td>
        <td id="LC794" class="blob-code js-file-line">            <span class="nx">i0</span> <span class="o">=</span> <span class="nx">width</span> <span class="o">-</span> <span class="nx">cubism_metricOverlap</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L795" class="blob-num js-line-number" data-line-number="795"></td>
        <td id="LC795" class="blob-code js-file-line">            <span class="kd">var</span> <span class="nx">dx</span> <span class="o">=</span> <span class="p">(</span><span class="nx">start1</span> <span class="o">-</span> <span class="nx">start</span><span class="p">)</span> <span class="o">/</span> <span class="nx">step</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L796" class="blob-num js-line-number" data-line-number="796"></td>
        <td id="LC796" class="blob-code js-file-line">            <span class="k">if</span> <span class="p">(</span><span class="nx">dx</span> <span class="o">&lt;</span> <span class="nx">width</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L797" class="blob-num js-line-number" data-line-number="797"></td>
        <td id="LC797" class="blob-code js-file-line">              <span class="kd">var</span> <span class="nx">canvas0</span> <span class="o">=</span> <span class="nx">buffer</span><span class="p">.</span><span class="nx">getContext</span><span class="p">(</span><span class="s2">&quot;2d&quot;</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L798" class="blob-num js-line-number" data-line-number="798"></td>
        <td id="LC798" class="blob-code js-file-line">              <span class="nx">canvas0</span><span class="p">.</span><span class="nx">clearRect</span><span class="p">(</span><span class="mi">0</span><span class="p">,</span> <span class="mi">0</span><span class="p">,</span> <span class="nx">width</span><span class="p">,</span> <span class="nx">height</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L799" class="blob-num js-line-number" data-line-number="799"></td>
        <td id="LC799" class="blob-code js-file-line">              <span class="nx">canvas0</span><span class="p">.</span><span class="nx">drawImage</span><span class="p">(</span><span class="nx">canvas</span><span class="p">.</span><span class="nx">canvas</span><span class="p">,</span> <span class="nx">dx</span><span class="p">,</span> <span class="mi">0</span><span class="p">,</span> <span class="nx">width</span> <span class="o">-</span> <span class="nx">dx</span><span class="p">,</span> <span class="nx">height</span><span class="p">,</span> <span class="mi">0</span><span class="p">,</span> <span class="mi">0</span><span class="p">,</span> <span class="nx">width</span> <span class="o">-</span> <span class="nx">dx</span><span class="p">,</span> <span class="nx">height</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L800" class="blob-num js-line-number" data-line-number="800"></td>
        <td id="LC800" class="blob-code js-file-line">              <span class="nx">canvas</span><span class="p">.</span><span class="nx">clearRect</span><span class="p">(</span><span class="mi">0</span><span class="p">,</span> <span class="mi">0</span><span class="p">,</span> <span class="nx">width</span><span class="p">,</span> <span class="nx">height</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L801" class="blob-num js-line-number" data-line-number="801"></td>
        <td id="LC801" class="blob-code js-file-line">              <span class="nx">canvas</span><span class="p">.</span><span class="nx">drawImage</span><span class="p">(</span><span class="nx">canvas0</span><span class="p">.</span><span class="nx">canvas</span><span class="p">,</span> <span class="mi">0</span><span class="p">,</span> <span class="mi">0</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L802" class="blob-num js-line-number" data-line-number="802"></td>
        <td id="LC802" class="blob-code js-file-line">            <span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L803" class="blob-num js-line-number" data-line-number="803"></td>
        <td id="LC803" class="blob-code js-file-line">          <span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L804" class="blob-num js-line-number" data-line-number="804"></td>
        <td id="LC804" class="blob-code js-file-line">          <span class="nx">start</span> <span class="o">=</span> <span class="nx">start1</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L805" class="blob-num js-line-number" data-line-number="805"></td>
        <td id="LC805" class="blob-code js-file-line">        <span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L806" class="blob-num js-line-number" data-line-number="806"></td>
        <td id="LC806" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L807" class="blob-num js-line-number" data-line-number="807"></td>
        <td id="LC807" class="blob-code js-file-line">        <span class="c1">// update the domain</span></td>
      </tr>
      <tr>
        <td id="L808" class="blob-num js-line-number" data-line-number="808"></td>
        <td id="LC808" class="blob-code js-file-line">        <span class="nx">scale</span><span class="p">.</span><span class="nx">domain</span><span class="p">([</span><span class="mi">0</span><span class="p">,</span> <span class="nx">max_</span> <span class="o">=</span> <span class="nx">max</span><span class="p">]);</span></td>
      </tr>
      <tr>
        <td id="L809" class="blob-num js-line-number" data-line-number="809"></td>
        <td id="LC809" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L810" class="blob-num js-line-number" data-line-number="810"></td>
        <td id="LC810" class="blob-code js-file-line">        <span class="c1">// clear for the new data</span></td>
      </tr>
      <tr>
        <td id="L811" class="blob-num js-line-number" data-line-number="811"></td>
        <td id="LC811" class="blob-code js-file-line">        <span class="nx">canvas</span><span class="p">.</span><span class="nx">clearRect</span><span class="p">(</span><span class="nx">i0</span><span class="p">,</span> <span class="mi">0</span><span class="p">,</span> <span class="nx">width</span> <span class="o">-</span> <span class="nx">i0</span><span class="p">,</span> <span class="nx">height</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L812" class="blob-num js-line-number" data-line-number="812"></td>
        <td id="LC812" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L813" class="blob-num js-line-number" data-line-number="813"></td>
        <td id="LC813" class="blob-code js-file-line">        <span class="c1">// record whether there are negative values to display</span></td>
      </tr>
      <tr>
        <td id="L814" class="blob-num js-line-number" data-line-number="814"></td>
        <td id="LC814" class="blob-code js-file-line">        <span class="kd">var</span> <span class="nx">negative</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L815" class="blob-num js-line-number" data-line-number="815"></td>
        <td id="LC815" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L816" class="blob-num js-line-number" data-line-number="816"></td>
        <td id="LC816" class="blob-code js-file-line">        <span class="c1">// positive bands</span></td>
      </tr>
      <tr>
        <td id="L817" class="blob-num js-line-number" data-line-number="817"></td>
        <td id="LC817" class="blob-code js-file-line">        <span class="k">for</span> <span class="p">(</span><span class="kd">var</span> <span class="nx">j</span> <span class="o">=</span> <span class="mi">0</span><span class="p">;</span> <span class="nx">j</span> <span class="o">&lt;</span> <span class="nx">m</span><span class="p">;</span> <span class="o">++</span><span class="nx">j</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L818" class="blob-num js-line-number" data-line-number="818"></td>
        <td id="LC818" class="blob-code js-file-line">          <span class="nx">canvas</span><span class="p">.</span><span class="nx">fillStyle</span> <span class="o">=</span> <span class="nx">colors_</span><span class="p">[</span><span class="nx">m</span> <span class="o">+</span> <span class="nx">j</span><span class="p">];</span></td>
      </tr>
      <tr>
        <td id="L819" class="blob-num js-line-number" data-line-number="819"></td>
        <td id="LC819" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L820" class="blob-num js-line-number" data-line-number="820"></td>
        <td id="LC820" class="blob-code js-file-line">          <span class="c1">// Adjust the range based on the current band index.</span></td>
      </tr>
      <tr>
        <td id="L821" class="blob-num js-line-number" data-line-number="821"></td>
        <td id="LC821" class="blob-code js-file-line">          <span class="kd">var</span> <span class="nx">y0</span> <span class="o">=</span> <span class="p">(</span><span class="nx">j</span> <span class="o">-</span> <span class="nx">m</span> <span class="o">+</span> <span class="mi">1</span><span class="p">)</span> <span class="o">*</span> <span class="nx">height</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L822" class="blob-num js-line-number" data-line-number="822"></td>
        <td id="LC822" class="blob-code js-file-line">          <span class="nx">scale</span><span class="p">.</span><span class="nx">range</span><span class="p">([</span><span class="nx">m</span> <span class="o">*</span> <span class="nx">height</span> <span class="o">+</span> <span class="nx">y0</span><span class="p">,</span> <span class="nx">y0</span><span class="p">]);</span></td>
      </tr>
      <tr>
        <td id="L823" class="blob-num js-line-number" data-line-number="823"></td>
        <td id="LC823" class="blob-code js-file-line">          <span class="nx">y0</span> <span class="o">=</span> <span class="nx">scale</span><span class="p">(</span><span class="mi">0</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L824" class="blob-num js-line-number" data-line-number="824"></td>
        <td id="LC824" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L825" class="blob-num js-line-number" data-line-number="825"></td>
        <td id="LC825" class="blob-code js-file-line">          <span class="k">for</span> <span class="p">(</span><span class="kd">var</span> <span class="nx">i</span> <span class="o">=</span> <span class="nx">i0</span><span class="p">,</span> <span class="nx">n</span> <span class="o">=</span> <span class="nx">width</span><span class="p">,</span> <span class="nx">y1</span><span class="p">;</span> <span class="nx">i</span> <span class="o">&lt;</span> <span class="nx">n</span><span class="p">;</span> <span class="o">++</span><span class="nx">i</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L826" class="blob-num js-line-number" data-line-number="826"></td>
        <td id="LC826" class="blob-code js-file-line">            <span class="nx">y1</span> <span class="o">=</span> <span class="nx">metric_</span><span class="p">.</span><span class="nx">valueAt</span><span class="p">(</span><span class="nx">i</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L827" class="blob-num js-line-number" data-line-number="827"></td>
        <td id="LC827" class="blob-code js-file-line">            <span class="k">if</span> <span class="p">(</span><span class="nx">y1</span> <span class="o">&lt;=</span> <span class="mi">0</span><span class="p">)</span> <span class="p">{</span> <span class="nx">negative</span> <span class="o">=</span> <span class="kc">true</span><span class="p">;</span> <span class="k">continue</span><span class="p">;</span> <span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L828" class="blob-num js-line-number" data-line-number="828"></td>
        <td id="LC828" class="blob-code js-file-line">            <span class="k">if</span> <span class="p">(</span><span class="nx">y1</span> <span class="o">===</span> <span class="kc">undefined</span><span class="p">)</span> <span class="k">continue</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L829" class="blob-num js-line-number" data-line-number="829"></td>
        <td id="LC829" class="blob-code js-file-line">            <span class="nx">canvas</span><span class="p">.</span><span class="nx">fillRect</span><span class="p">(</span><span class="nx">i</span><span class="p">,</span> <span class="nx">y1</span> <span class="o">=</span> <span class="nx">scale</span><span class="p">(</span><span class="nx">y1</span><span class="p">),</span> <span class="mi">1</span><span class="p">,</span> <span class="nx">y0</span> <span class="o">-</span> <span class="nx">y1</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L830" class="blob-num js-line-number" data-line-number="830"></td>
        <td id="LC830" class="blob-code js-file-line">          <span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L831" class="blob-num js-line-number" data-line-number="831"></td>
        <td id="LC831" class="blob-code js-file-line">        <span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L832" class="blob-num js-line-number" data-line-number="832"></td>
        <td id="LC832" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L833" class="blob-num js-line-number" data-line-number="833"></td>
        <td id="LC833" class="blob-code js-file-line">        <span class="k">if</span> <span class="p">(</span><span class="nx">negative</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L834" class="blob-num js-line-number" data-line-number="834"></td>
        <td id="LC834" class="blob-code js-file-line">          <span class="c1">// enable offset mode</span></td>
      </tr>
      <tr>
        <td id="L835" class="blob-num js-line-number" data-line-number="835"></td>
        <td id="LC835" class="blob-code js-file-line">          <span class="k">if</span> <span class="p">(</span><span class="nx">mode</span> <span class="o">===</span> <span class="s2">&quot;offset&quot;</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L836" class="blob-num js-line-number" data-line-number="836"></td>
        <td id="LC836" class="blob-code js-file-line">            <span class="nx">canvas</span><span class="p">.</span><span class="nx">translate</span><span class="p">(</span><span class="mi">0</span><span class="p">,</span> <span class="nx">height</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L837" class="blob-num js-line-number" data-line-number="837"></td>
        <td id="LC837" class="blob-code js-file-line">            <span class="nx">canvas</span><span class="p">.</span><span class="nx">scale</span><span class="p">(</span><span class="mi">1</span><span class="p">,</span> <span class="o">-</span><span class="mi">1</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L838" class="blob-num js-line-number" data-line-number="838"></td>
        <td id="LC838" class="blob-code js-file-line">          <span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L839" class="blob-num js-line-number" data-line-number="839"></td>
        <td id="LC839" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L840" class="blob-num js-line-number" data-line-number="840"></td>
        <td id="LC840" class="blob-code js-file-line">          <span class="c1">// negative bands</span></td>
      </tr>
      <tr>
        <td id="L841" class="blob-num js-line-number" data-line-number="841"></td>
        <td id="LC841" class="blob-code js-file-line">          <span class="k">for</span> <span class="p">(</span><span class="kd">var</span> <span class="nx">j</span> <span class="o">=</span> <span class="mi">0</span><span class="p">;</span> <span class="nx">j</span> <span class="o">&lt;</span> <span class="nx">m</span><span class="p">;</span> <span class="o">++</span><span class="nx">j</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L842" class="blob-num js-line-number" data-line-number="842"></td>
        <td id="LC842" class="blob-code js-file-line">            <span class="nx">canvas</span><span class="p">.</span><span class="nx">fillStyle</span> <span class="o">=</span> <span class="nx">colors_</span><span class="p">[</span><span class="nx">m</span> <span class="o">-</span> <span class="mi">1</span> <span class="o">-</span> <span class="nx">j</span><span class="p">];</span></td>
      </tr>
      <tr>
        <td id="L843" class="blob-num js-line-number" data-line-number="843"></td>
        <td id="LC843" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L844" class="blob-num js-line-number" data-line-number="844"></td>
        <td id="LC844" class="blob-code js-file-line">            <span class="c1">// Adjust the range based on the current band index.</span></td>
      </tr>
      <tr>
        <td id="L845" class="blob-num js-line-number" data-line-number="845"></td>
        <td id="LC845" class="blob-code js-file-line">            <span class="kd">var</span> <span class="nx">y0</span> <span class="o">=</span> <span class="p">(</span><span class="nx">j</span> <span class="o">-</span> <span class="nx">m</span> <span class="o">+</span> <span class="mi">1</span><span class="p">)</span> <span class="o">*</span> <span class="nx">height</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L846" class="blob-num js-line-number" data-line-number="846"></td>
        <td id="LC846" class="blob-code js-file-line">            <span class="nx">scale</span><span class="p">.</span><span class="nx">range</span><span class="p">([</span><span class="nx">m</span> <span class="o">*</span> <span class="nx">height</span> <span class="o">+</span> <span class="nx">y0</span><span class="p">,</span> <span class="nx">y0</span><span class="p">]);</span></td>
      </tr>
      <tr>
        <td id="L847" class="blob-num js-line-number" data-line-number="847"></td>
        <td id="LC847" class="blob-code js-file-line">            <span class="nx">y0</span> <span class="o">=</span> <span class="nx">scale</span><span class="p">(</span><span class="mi">0</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L848" class="blob-num js-line-number" data-line-number="848"></td>
        <td id="LC848" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L849" class="blob-num js-line-number" data-line-number="849"></td>
        <td id="LC849" class="blob-code js-file-line">            <span class="k">for</span> <span class="p">(</span><span class="kd">var</span> <span class="nx">i</span> <span class="o">=</span> <span class="nx">i0</span><span class="p">,</span> <span class="nx">n</span> <span class="o">=</span> <span class="nx">width</span><span class="p">,</span> <span class="nx">y1</span><span class="p">;</span> <span class="nx">i</span> <span class="o">&lt;</span> <span class="nx">n</span><span class="p">;</span> <span class="o">++</span><span class="nx">i</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L850" class="blob-num js-line-number" data-line-number="850"></td>
        <td id="LC850" class="blob-code js-file-line">              <span class="nx">y1</span> <span class="o">=</span> <span class="nx">metric_</span><span class="p">.</span><span class="nx">valueAt</span><span class="p">(</span><span class="nx">i</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L851" class="blob-num js-line-number" data-line-number="851"></td>
        <td id="LC851" class="blob-code js-file-line">              <span class="k">if</span> <span class="p">(</span><span class="nx">y1</span> <span class="o">&gt;=</span> <span class="mi">0</span><span class="p">)</span> <span class="k">continue</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L852" class="blob-num js-line-number" data-line-number="852"></td>
        <td id="LC852" class="blob-code js-file-line">              <span class="nx">canvas</span><span class="p">.</span><span class="nx">fillRect</span><span class="p">(</span><span class="nx">i</span><span class="p">,</span> <span class="nx">scale</span><span class="p">(</span><span class="o">-</span><span class="nx">y1</span><span class="p">),</span> <span class="mi">1</span><span class="p">,</span> <span class="nx">y0</span> <span class="o">-</span> <span class="nx">scale</span><span class="p">(</span><span class="o">-</span><span class="nx">y1</span><span class="p">));</span></td>
      </tr>
      <tr>
        <td id="L853" class="blob-num js-line-number" data-line-number="853"></td>
        <td id="LC853" class="blob-code js-file-line">            <span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L854" class="blob-num js-line-number" data-line-number="854"></td>
        <td id="LC854" class="blob-code js-file-line">          <span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L855" class="blob-num js-line-number" data-line-number="855"></td>
        <td id="LC855" class="blob-code js-file-line">        <span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L856" class="blob-num js-line-number" data-line-number="856"></td>
        <td id="LC856" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L857" class="blob-num js-line-number" data-line-number="857"></td>
        <td id="LC857" class="blob-code js-file-line">        <span class="nx">canvas</span><span class="p">.</span><span class="nx">restore</span><span class="p">();</span></td>
      </tr>
      <tr>
        <td id="L858" class="blob-num js-line-number" data-line-number="858"></td>
        <td id="LC858" class="blob-code js-file-line">      <span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L859" class="blob-num js-line-number" data-line-number="859"></td>
        <td id="LC859" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L860" class="blob-num js-line-number" data-line-number="860"></td>
        <td id="LC860" class="blob-code js-file-line">      <span class="kd">function</span> <span class="nx">focus</span><span class="p">(</span><span class="nx">i</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L861" class="blob-num js-line-number" data-line-number="861"></td>
        <td id="LC861" class="blob-code js-file-line">        <span class="k">if</span> <span class="p">(</span><span class="nx">i</span> <span class="o">==</span> <span class="kc">null</span><span class="p">)</span> <span class="nx">i</span> <span class="o">=</span> <span class="nx">width</span> <span class="o">-</span> <span class="mi">1</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L862" class="blob-num js-line-number" data-line-number="862"></td>
        <td id="LC862" class="blob-code js-file-line">        <span class="kd">var</span> <span class="nx">value</span> <span class="o">=</span> <span class="nx">metric_</span><span class="p">.</span><span class="nx">valueAt</span><span class="p">(</span><span class="nx">i</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L863" class="blob-num js-line-number" data-line-number="863"></td>
        <td id="LC863" class="blob-code js-file-line">        <span class="nx">span</span><span class="p">.</span><span class="nx">datum</span><span class="p">(</span><span class="nx">value</span><span class="p">).</span><span class="nx">text</span><span class="p">(</span><span class="nb">isNaN</span><span class="p">(</span><span class="nx">value</span><span class="p">)</span> <span class="o">?</span> <span class="kc">null</span> <span class="o">:</span> <span class="nx">format</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L864" class="blob-num js-line-number" data-line-number="864"></td>
        <td id="LC864" class="blob-code js-file-line">      <span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L865" class="blob-num js-line-number" data-line-number="865"></td>
        <td id="LC865" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L866" class="blob-num js-line-number" data-line-number="866"></td>
        <td id="LC866" class="blob-code js-file-line">      <span class="c1">// Update the chart when the context changes.</span></td>
      </tr>
      <tr>
        <td id="L867" class="blob-num js-line-number" data-line-number="867"></td>
        <td id="LC867" class="blob-code js-file-line">      <span class="nx">context</span><span class="p">.</span><span class="nx">on</span><span class="p">(</span><span class="s2">&quot;change.horizon-&quot;</span> <span class="o">+</span> <span class="nx">id</span><span class="p">,</span> <span class="nx">change</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L868" class="blob-num js-line-number" data-line-number="868"></td>
        <td id="LC868" class="blob-code js-file-line">      <span class="nx">context</span><span class="p">.</span><span class="nx">on</span><span class="p">(</span><span class="s2">&quot;focus.horizon-&quot;</span> <span class="o">+</span> <span class="nx">id</span><span class="p">,</span> <span class="nx">focus</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L869" class="blob-num js-line-number" data-line-number="869"></td>
        <td id="LC869" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L870" class="blob-num js-line-number" data-line-number="870"></td>
        <td id="LC870" class="blob-code js-file-line">      <span class="c1">// Display the first metric change immediately,</span></td>
      </tr>
      <tr>
        <td id="L871" class="blob-num js-line-number" data-line-number="871"></td>
        <td id="LC871" class="blob-code js-file-line">      <span class="c1">// but defer subsequent updates to the canvas change.</span></td>
      </tr>
      <tr>
        <td id="L872" class="blob-num js-line-number" data-line-number="872"></td>
        <td id="LC872" class="blob-code js-file-line">      <span class="c1">// Note that someone still needs to listen to the metric,</span></td>
      </tr>
      <tr>
        <td id="L873" class="blob-num js-line-number" data-line-number="873"></td>
        <td id="LC873" class="blob-code js-file-line">      <span class="c1">// so that it continues to update automatically.</span></td>
      </tr>
      <tr>
        <td id="L874" class="blob-num js-line-number" data-line-number="874"></td>
        <td id="LC874" class="blob-code js-file-line">      <span class="nx">metric_</span><span class="p">.</span><span class="nx">on</span><span class="p">(</span><span class="s2">&quot;change.horizon-&quot;</span> <span class="o">+</span> <span class="nx">id</span><span class="p">,</span> <span class="kd">function</span><span class="p">(</span><span class="nx">start</span><span class="p">,</span> <span class="nx">stop</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L875" class="blob-num js-line-number" data-line-number="875"></td>
        <td id="LC875" class="blob-code js-file-line">        <span class="nx">change</span><span class="p">(</span><span class="nx">start</span><span class="p">,</span> <span class="nx">stop</span><span class="p">),</span> <span class="nx">focus</span><span class="p">();</span></td>
      </tr>
      <tr>
        <td id="L876" class="blob-num js-line-number" data-line-number="876"></td>
        <td id="LC876" class="blob-code js-file-line">        <span class="k">if</span> <span class="p">(</span><span class="nx">ready</span><span class="p">)</span> <span class="nx">metric_</span><span class="p">.</span><span class="nx">on</span><span class="p">(</span><span class="s2">&quot;change.horizon-&quot;</span> <span class="o">+</span> <span class="nx">id</span><span class="p">,</span> <span class="nx">cubism_identity</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L877" class="blob-num js-line-number" data-line-number="877"></td>
        <td id="LC877" class="blob-code js-file-line">      <span class="p">});</span></td>
      </tr>
      <tr>
        <td id="L878" class="blob-num js-line-number" data-line-number="878"></td>
        <td id="LC878" class="blob-code js-file-line">    <span class="p">});</span></td>
      </tr>
      <tr>
        <td id="L879" class="blob-num js-line-number" data-line-number="879"></td>
        <td id="LC879" class="blob-code js-file-line">  <span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L880" class="blob-num js-line-number" data-line-number="880"></td>
        <td id="LC880" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L881" class="blob-num js-line-number" data-line-number="881"></td>
        <td id="LC881" class="blob-code js-file-line">  <span class="nx">horizon</span><span class="p">.</span><span class="nx">remove</span> <span class="o">=</span> <span class="kd">function</span><span class="p">(</span><span class="nx">selection</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L882" class="blob-num js-line-number" data-line-number="882"></td>
        <td id="LC882" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L883" class="blob-num js-line-number" data-line-number="883"></td>
        <td id="LC883" class="blob-code js-file-line">    <span class="nx">selection</span></td>
      </tr>
      <tr>
        <td id="L884" class="blob-num js-line-number" data-line-number="884"></td>
        <td id="LC884" class="blob-code js-file-line">        <span class="p">.</span><span class="nx">on</span><span class="p">(</span><span class="s2">&quot;mousemove.horizon&quot;</span><span class="p">,</span> <span class="kc">null</span><span class="p">)</span></td>
      </tr>
      <tr>
        <td id="L885" class="blob-num js-line-number" data-line-number="885"></td>
        <td id="LC885" class="blob-code js-file-line">        <span class="p">.</span><span class="nx">on</span><span class="p">(</span><span class="s2">&quot;mouseout.horizon&quot;</span><span class="p">,</span> <span class="kc">null</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L886" class="blob-num js-line-number" data-line-number="886"></td>
        <td id="LC886" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L887" class="blob-num js-line-number" data-line-number="887"></td>
        <td id="LC887" class="blob-code js-file-line">    <span class="nx">selection</span><span class="p">.</span><span class="nx">selectAll</span><span class="p">(</span><span class="s2">&quot;canvas&quot;</span><span class="p">)</span></td>
      </tr>
      <tr>
        <td id="L888" class="blob-num js-line-number" data-line-number="888"></td>
        <td id="LC888" class="blob-code js-file-line">        <span class="p">.</span><span class="nx">each</span><span class="p">(</span><span class="nx">remove</span><span class="p">)</span></td>
      </tr>
      <tr>
        <td id="L889" class="blob-num js-line-number" data-line-number="889"></td>
        <td id="LC889" class="blob-code js-file-line">        <span class="p">.</span><span class="nx">remove</span><span class="p">();</span></td>
      </tr>
      <tr>
        <td id="L890" class="blob-num js-line-number" data-line-number="890"></td>
        <td id="LC890" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L891" class="blob-num js-line-number" data-line-number="891"></td>
        <td id="LC891" class="blob-code js-file-line">    <span class="nx">selection</span><span class="p">.</span><span class="nx">selectAll</span><span class="p">(</span><span class="s2">&quot;.title,.value&quot;</span><span class="p">)</span></td>
      </tr>
      <tr>
        <td id="L892" class="blob-num js-line-number" data-line-number="892"></td>
        <td id="LC892" class="blob-code js-file-line">        <span class="p">.</span><span class="nx">remove</span><span class="p">();</span></td>
      </tr>
      <tr>
        <td id="L893" class="blob-num js-line-number" data-line-number="893"></td>
        <td id="LC893" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L894" class="blob-num js-line-number" data-line-number="894"></td>
        <td id="LC894" class="blob-code js-file-line">    <span class="kd">function</span> <span class="nx">remove</span><span class="p">(</span><span class="nx">d</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L895" class="blob-num js-line-number" data-line-number="895"></td>
        <td id="LC895" class="blob-code js-file-line">      <span class="nx">d</span><span class="p">.</span><span class="nx">metric</span><span class="p">.</span><span class="nx">on</span><span class="p">(</span><span class="s2">&quot;change.horizon-&quot;</span> <span class="o">+</span> <span class="nx">d</span><span class="p">.</span><span class="nx">id</span><span class="p">,</span> <span class="kc">null</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L896" class="blob-num js-line-number" data-line-number="896"></td>
        <td id="LC896" class="blob-code js-file-line">      <span class="nx">context</span><span class="p">.</span><span class="nx">on</span><span class="p">(</span><span class="s2">&quot;change.horizon-&quot;</span> <span class="o">+</span> <span class="nx">d</span><span class="p">.</span><span class="nx">id</span><span class="p">,</span> <span class="kc">null</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L897" class="blob-num js-line-number" data-line-number="897"></td>
        <td id="LC897" class="blob-code js-file-line">      <span class="nx">context</span><span class="p">.</span><span class="nx">on</span><span class="p">(</span><span class="s2">&quot;focus.horizon-&quot;</span> <span class="o">+</span> <span class="nx">d</span><span class="p">.</span><span class="nx">id</span><span class="p">,</span> <span class="kc">null</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L898" class="blob-num js-line-number" data-line-number="898"></td>
        <td id="LC898" class="blob-code js-file-line">    <span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L899" class="blob-num js-line-number" data-line-number="899"></td>
        <td id="LC899" class="blob-code js-file-line">  <span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L900" class="blob-num js-line-number" data-line-number="900"></td>
        <td id="LC900" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L901" class="blob-num js-line-number" data-line-number="901"></td>
        <td id="LC901" class="blob-code js-file-line">  <span class="nx">horizon</span><span class="p">.</span><span class="nx">mode</span> <span class="o">=</span> <span class="kd">function</span><span class="p">(</span><span class="nx">_</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L902" class="blob-num js-line-number" data-line-number="902"></td>
        <td id="LC902" class="blob-code js-file-line">    <span class="k">if</span> <span class="p">(</span><span class="o">!</span><span class="nx">arguments</span><span class="p">.</span><span class="nx">length</span><span class="p">)</span> <span class="k">return</span> <span class="nx">mode</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L903" class="blob-num js-line-number" data-line-number="903"></td>
        <td id="LC903" class="blob-code js-file-line">    <span class="nx">mode</span> <span class="o">=</span> <span class="nx">_</span> <span class="o">+</span> <span class="s2">&quot;&quot;</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L904" class="blob-num js-line-number" data-line-number="904"></td>
        <td id="LC904" class="blob-code js-file-line">    <span class="k">return</span> <span class="nx">horizon</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L905" class="blob-num js-line-number" data-line-number="905"></td>
        <td id="LC905" class="blob-code js-file-line">  <span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L906" class="blob-num js-line-number" data-line-number="906"></td>
        <td id="LC906" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L907" class="blob-num js-line-number" data-line-number="907"></td>
        <td id="LC907" class="blob-code js-file-line">  <span class="nx">horizon</span><span class="p">.</span><span class="nx">height</span> <span class="o">=</span> <span class="kd">function</span><span class="p">(</span><span class="nx">_</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L908" class="blob-num js-line-number" data-line-number="908"></td>
        <td id="LC908" class="blob-code js-file-line">    <span class="k">if</span> <span class="p">(</span><span class="o">!</span><span class="nx">arguments</span><span class="p">.</span><span class="nx">length</span><span class="p">)</span> <span class="k">return</span> <span class="nx">height</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L909" class="blob-num js-line-number" data-line-number="909"></td>
        <td id="LC909" class="blob-code js-file-line">    <span class="nx">buffer</span><span class="p">.</span><span class="nx">height</span> <span class="o">=</span> <span class="nx">height</span> <span class="o">=</span> <span class="o">+</span><span class="nx">_</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L910" class="blob-num js-line-number" data-line-number="910"></td>
        <td id="LC910" class="blob-code js-file-line">    <span class="k">return</span> <span class="nx">horizon</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L911" class="blob-num js-line-number" data-line-number="911"></td>
        <td id="LC911" class="blob-code js-file-line">  <span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L912" class="blob-num js-line-number" data-line-number="912"></td>
        <td id="LC912" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L913" class="blob-num js-line-number" data-line-number="913"></td>
        <td id="LC913" class="blob-code js-file-line">  <span class="nx">horizon</span><span class="p">.</span><span class="nx">metric</span> <span class="o">=</span> <span class="kd">function</span><span class="p">(</span><span class="nx">_</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L914" class="blob-num js-line-number" data-line-number="914"></td>
        <td id="LC914" class="blob-code js-file-line">    <span class="k">if</span> <span class="p">(</span><span class="o">!</span><span class="nx">arguments</span><span class="p">.</span><span class="nx">length</span><span class="p">)</span> <span class="k">return</span> <span class="nx">metric</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L915" class="blob-num js-line-number" data-line-number="915"></td>
        <td id="LC915" class="blob-code js-file-line">    <span class="nx">metric</span> <span class="o">=</span> <span class="nx">_</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L916" class="blob-num js-line-number" data-line-number="916"></td>
        <td id="LC916" class="blob-code js-file-line">    <span class="k">return</span> <span class="nx">horizon</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L917" class="blob-num js-line-number" data-line-number="917"></td>
        <td id="LC917" class="blob-code js-file-line">  <span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L918" class="blob-num js-line-number" data-line-number="918"></td>
        <td id="LC918" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L919" class="blob-num js-line-number" data-line-number="919"></td>
        <td id="LC919" class="blob-code js-file-line">  <span class="nx">horizon</span><span class="p">.</span><span class="nx">scale</span> <span class="o">=</span> <span class="kd">function</span><span class="p">(</span><span class="nx">_</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L920" class="blob-num js-line-number" data-line-number="920"></td>
        <td id="LC920" class="blob-code js-file-line">    <span class="k">if</span> <span class="p">(</span><span class="o">!</span><span class="nx">arguments</span><span class="p">.</span><span class="nx">length</span><span class="p">)</span> <span class="k">return</span> <span class="nx">scale</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L921" class="blob-num js-line-number" data-line-number="921"></td>
        <td id="LC921" class="blob-code js-file-line">    <span class="nx">scale</span> <span class="o">=</span> <span class="nx">_</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L922" class="blob-num js-line-number" data-line-number="922"></td>
        <td id="LC922" class="blob-code js-file-line">    <span class="k">return</span> <span class="nx">horizon</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L923" class="blob-num js-line-number" data-line-number="923"></td>
        <td id="LC923" class="blob-code js-file-line">  <span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L924" class="blob-num js-line-number" data-line-number="924"></td>
        <td id="LC924" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L925" class="blob-num js-line-number" data-line-number="925"></td>
        <td id="LC925" class="blob-code js-file-line">  <span class="nx">horizon</span><span class="p">.</span><span class="nx">extent</span> <span class="o">=</span> <span class="kd">function</span><span class="p">(</span><span class="nx">_</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L926" class="blob-num js-line-number" data-line-number="926"></td>
        <td id="LC926" class="blob-code js-file-line">    <span class="k">if</span> <span class="p">(</span><span class="o">!</span><span class="nx">arguments</span><span class="p">.</span><span class="nx">length</span><span class="p">)</span> <span class="k">return</span> <span class="nx">extent</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L927" class="blob-num js-line-number" data-line-number="927"></td>
        <td id="LC927" class="blob-code js-file-line">    <span class="nx">extent</span> <span class="o">=</span> <span class="nx">_</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L928" class="blob-num js-line-number" data-line-number="928"></td>
        <td id="LC928" class="blob-code js-file-line">    <span class="k">return</span> <span class="nx">horizon</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L929" class="blob-num js-line-number" data-line-number="929"></td>
        <td id="LC929" class="blob-code js-file-line">  <span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L930" class="blob-num js-line-number" data-line-number="930"></td>
        <td id="LC930" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L931" class="blob-num js-line-number" data-line-number="931"></td>
        <td id="LC931" class="blob-code js-file-line">  <span class="nx">horizon</span><span class="p">.</span><span class="nx">title</span> <span class="o">=</span> <span class="kd">function</span><span class="p">(</span><span class="nx">_</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L932" class="blob-num js-line-number" data-line-number="932"></td>
        <td id="LC932" class="blob-code js-file-line">    <span class="k">if</span> <span class="p">(</span><span class="o">!</span><span class="nx">arguments</span><span class="p">.</span><span class="nx">length</span><span class="p">)</span> <span class="k">return</span> <span class="nx">title</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L933" class="blob-num js-line-number" data-line-number="933"></td>
        <td id="LC933" class="blob-code js-file-line">    <span class="nx">title</span> <span class="o">=</span> <span class="nx">_</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L934" class="blob-num js-line-number" data-line-number="934"></td>
        <td id="LC934" class="blob-code js-file-line">    <span class="k">return</span> <span class="nx">horizon</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L935" class="blob-num js-line-number" data-line-number="935"></td>
        <td id="LC935" class="blob-code js-file-line">  <span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L936" class="blob-num js-line-number" data-line-number="936"></td>
        <td id="LC936" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L937" class="blob-num js-line-number" data-line-number="937"></td>
        <td id="LC937" class="blob-code js-file-line">  <span class="nx">horizon</span><span class="p">.</span><span class="nx">format</span> <span class="o">=</span> <span class="kd">function</span><span class="p">(</span><span class="nx">_</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L938" class="blob-num js-line-number" data-line-number="938"></td>
        <td id="LC938" class="blob-code js-file-line">    <span class="k">if</span> <span class="p">(</span><span class="o">!</span><span class="nx">arguments</span><span class="p">.</span><span class="nx">length</span><span class="p">)</span> <span class="k">return</span> <span class="nx">format</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L939" class="blob-num js-line-number" data-line-number="939"></td>
        <td id="LC939" class="blob-code js-file-line">    <span class="nx">format</span> <span class="o">=</span> <span class="nx">_</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L940" class="blob-num js-line-number" data-line-number="940"></td>
        <td id="LC940" class="blob-code js-file-line">    <span class="k">return</span> <span class="nx">horizon</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L941" class="blob-num js-line-number" data-line-number="941"></td>
        <td id="LC941" class="blob-code js-file-line">  <span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L942" class="blob-num js-line-number" data-line-number="942"></td>
        <td id="LC942" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L943" class="blob-num js-line-number" data-line-number="943"></td>
        <td id="LC943" class="blob-code js-file-line">  <span class="nx">horizon</span><span class="p">.</span><span class="nx">colors</span> <span class="o">=</span> <span class="kd">function</span><span class="p">(</span><span class="nx">_</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L944" class="blob-num js-line-number" data-line-number="944"></td>
        <td id="LC944" class="blob-code js-file-line">    <span class="k">if</span> <span class="p">(</span><span class="o">!</span><span class="nx">arguments</span><span class="p">.</span><span class="nx">length</span><span class="p">)</span> <span class="k">return</span> <span class="nx">colors</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L945" class="blob-num js-line-number" data-line-number="945"></td>
        <td id="LC945" class="blob-code js-file-line">    <span class="nx">colors</span> <span class="o">=</span> <span class="nx">_</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L946" class="blob-num js-line-number" data-line-number="946"></td>
        <td id="LC946" class="blob-code js-file-line">    <span class="k">return</span> <span class="nx">horizon</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L947" class="blob-num js-line-number" data-line-number="947"></td>
        <td id="LC947" class="blob-code js-file-line">  <span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L948" class="blob-num js-line-number" data-line-number="948"></td>
        <td id="LC948" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L949" class="blob-num js-line-number" data-line-number="949"></td>
        <td id="LC949" class="blob-code js-file-line">  <span class="k">return</span> <span class="nx">horizon</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L950" class="blob-num js-line-number" data-line-number="950"></td>
        <td id="LC950" class="blob-code js-file-line"><span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L951" class="blob-num js-line-number" data-line-number="951"></td>
        <td id="LC951" class="blob-code js-file-line"><span class="nx">cubism_contextPrototype</span><span class="p">.</span><span class="nx">comparison</span> <span class="o">=</span> <span class="kd">function</span><span class="p">()</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L952" class="blob-num js-line-number" data-line-number="952"></td>
        <td id="LC952" class="blob-code js-file-line">  <span class="kd">var</span> <span class="nx">context</span> <span class="o">=</span> <span class="k">this</span><span class="p">,</span></td>
      </tr>
      <tr>
        <td id="L953" class="blob-num js-line-number" data-line-number="953"></td>
        <td id="LC953" class="blob-code js-file-line">      <span class="nx">width</span> <span class="o">=</span> <span class="nx">context</span><span class="p">.</span><span class="nx">size</span><span class="p">(),</span></td>
      </tr>
      <tr>
        <td id="L954" class="blob-num js-line-number" data-line-number="954"></td>
        <td id="LC954" class="blob-code js-file-line">      <span class="nx">height</span> <span class="o">=</span> <span class="mi">120</span><span class="p">,</span></td>
      </tr>
      <tr>
        <td id="L955" class="blob-num js-line-number" data-line-number="955"></td>
        <td id="LC955" class="blob-code js-file-line">      <span class="nx">scale</span> <span class="o">=</span> <span class="nx">d3</span><span class="p">.</span><span class="nx">scale</span><span class="p">.</span><span class="nx">linear</span><span class="p">().</span><span class="nx">interpolate</span><span class="p">(</span><span class="nx">d3</span><span class="p">.</span><span class="nx">interpolateRound</span><span class="p">),</span></td>
      </tr>
      <tr>
        <td id="L956" class="blob-num js-line-number" data-line-number="956"></td>
        <td id="LC956" class="blob-code js-file-line">      <span class="nx">primary</span> <span class="o">=</span> <span class="kd">function</span><span class="p">(</span><span class="nx">d</span><span class="p">)</span> <span class="p">{</span> <span class="k">return</span> <span class="nx">d</span><span class="p">[</span><span class="mi">0</span><span class="p">];</span> <span class="p">},</span></td>
      </tr>
      <tr>
        <td id="L957" class="blob-num js-line-number" data-line-number="957"></td>
        <td id="LC957" class="blob-code js-file-line">      <span class="nx">secondary</span> <span class="o">=</span> <span class="kd">function</span><span class="p">(</span><span class="nx">d</span><span class="p">)</span> <span class="p">{</span> <span class="k">return</span> <span class="nx">d</span><span class="p">[</span><span class="mi">1</span><span class="p">];</span> <span class="p">},</span></td>
      </tr>
      <tr>
        <td id="L958" class="blob-num js-line-number" data-line-number="958"></td>
        <td id="LC958" class="blob-code js-file-line">      <span class="nx">extent</span> <span class="o">=</span> <span class="kc">null</span><span class="p">,</span></td>
      </tr>
      <tr>
        <td id="L959" class="blob-num js-line-number" data-line-number="959"></td>
        <td id="LC959" class="blob-code js-file-line">      <span class="nx">title</span> <span class="o">=</span> <span class="nx">cubism_identity</span><span class="p">,</span></td>
      </tr>
      <tr>
        <td id="L960" class="blob-num js-line-number" data-line-number="960"></td>
        <td id="LC960" class="blob-code js-file-line">      <span class="nx">formatPrimary</span> <span class="o">=</span> <span class="nx">cubism_comparisonPrimaryFormat</span><span class="p">,</span></td>
      </tr>
      <tr>
        <td id="L961" class="blob-num js-line-number" data-line-number="961"></td>
        <td id="LC961" class="blob-code js-file-line">      <span class="nx">formatChange</span> <span class="o">=</span> <span class="nx">cubism_comparisonChangeFormat</span><span class="p">,</span></td>
      </tr>
      <tr>
        <td id="L962" class="blob-num js-line-number" data-line-number="962"></td>
        <td id="LC962" class="blob-code js-file-line">      <span class="nx">colors</span> <span class="o">=</span> <span class="p">[</span><span class="s2">&quot;#9ecae1&quot;</span><span class="p">,</span> <span class="s2">&quot;#225b84&quot;</span><span class="p">,</span> <span class="s2">&quot;#a1d99b&quot;</span><span class="p">,</span> <span class="s2">&quot;#22723a&quot;</span><span class="p">],</span></td>
      </tr>
      <tr>
        <td id="L963" class="blob-num js-line-number" data-line-number="963"></td>
        <td id="LC963" class="blob-code js-file-line">      <span class="nx">strokeWidth</span> <span class="o">=</span> <span class="mf">1.5</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L964" class="blob-num js-line-number" data-line-number="964"></td>
        <td id="LC964" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L965" class="blob-num js-line-number" data-line-number="965"></td>
        <td id="LC965" class="blob-code js-file-line">  <span class="kd">function</span> <span class="nx">comparison</span><span class="p">(</span><span class="nx">selection</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L966" class="blob-num js-line-number" data-line-number="966"></td>
        <td id="LC966" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L967" class="blob-num js-line-number" data-line-number="967"></td>
        <td id="LC967" class="blob-code js-file-line">    <span class="nx">selection</span></td>
      </tr>
      <tr>
        <td id="L968" class="blob-num js-line-number" data-line-number="968"></td>
        <td id="LC968" class="blob-code js-file-line">        <span class="p">.</span><span class="nx">on</span><span class="p">(</span><span class="s2">&quot;mousemove.comparison&quot;</span><span class="p">,</span> <span class="kd">function</span><span class="p">()</span> <span class="p">{</span> <span class="nx">context</span><span class="p">.</span><span class="nx">focus</span><span class="p">(</span><span class="nb">Math</span><span class="p">.</span><span class="nx">round</span><span class="p">(</span><span class="nx">d3</span><span class="p">.</span><span class="nx">mouse</span><span class="p">(</span><span class="k">this</span><span class="p">)[</span><span class="mi">0</span><span class="p">]));</span> <span class="p">})</span></td>
      </tr>
      <tr>
        <td id="L969" class="blob-num js-line-number" data-line-number="969"></td>
        <td id="LC969" class="blob-code js-file-line">        <span class="p">.</span><span class="nx">on</span><span class="p">(</span><span class="s2">&quot;mouseout.comparison&quot;</span><span class="p">,</span> <span class="kd">function</span><span class="p">()</span> <span class="p">{</span> <span class="nx">context</span><span class="p">.</span><span class="nx">focus</span><span class="p">(</span><span class="kc">null</span><span class="p">);</span> <span class="p">});</span></td>
      </tr>
      <tr>
        <td id="L970" class="blob-num js-line-number" data-line-number="970"></td>
        <td id="LC970" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L971" class="blob-num js-line-number" data-line-number="971"></td>
        <td id="LC971" class="blob-code js-file-line">    <span class="nx">selection</span><span class="p">.</span><span class="nx">append</span><span class="p">(</span><span class="s2">&quot;canvas&quot;</span><span class="p">)</span></td>
      </tr>
      <tr>
        <td id="L972" class="blob-num js-line-number" data-line-number="972"></td>
        <td id="LC972" class="blob-code js-file-line">        <span class="p">.</span><span class="nx">attr</span><span class="p">(</span><span class="s2">&quot;width&quot;</span><span class="p">,</span> <span class="nx">width</span><span class="p">)</span></td>
      </tr>
      <tr>
        <td id="L973" class="blob-num js-line-number" data-line-number="973"></td>
        <td id="LC973" class="blob-code js-file-line">        <span class="p">.</span><span class="nx">attr</span><span class="p">(</span><span class="s2">&quot;height&quot;</span><span class="p">,</span> <span class="nx">height</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L974" class="blob-num js-line-number" data-line-number="974"></td>
        <td id="LC974" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L975" class="blob-num js-line-number" data-line-number="975"></td>
        <td id="LC975" class="blob-code js-file-line">    <span class="nx">selection</span><span class="p">.</span><span class="nx">append</span><span class="p">(</span><span class="s2">&quot;span&quot;</span><span class="p">)</span></td>
      </tr>
      <tr>
        <td id="L976" class="blob-num js-line-number" data-line-number="976"></td>
        <td id="LC976" class="blob-code js-file-line">        <span class="p">.</span><span class="nx">attr</span><span class="p">(</span><span class="s2">&quot;class&quot;</span><span class="p">,</span> <span class="s2">&quot;title&quot;</span><span class="p">)</span></td>
      </tr>
      <tr>
        <td id="L977" class="blob-num js-line-number" data-line-number="977"></td>
        <td id="LC977" class="blob-code js-file-line">        <span class="p">.</span><span class="nx">text</span><span class="p">(</span><span class="nx">title</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L978" class="blob-num js-line-number" data-line-number="978"></td>
        <td id="LC978" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L979" class="blob-num js-line-number" data-line-number="979"></td>
        <td id="LC979" class="blob-code js-file-line">    <span class="nx">selection</span><span class="p">.</span><span class="nx">append</span><span class="p">(</span><span class="s2">&quot;span&quot;</span><span class="p">)</span></td>
      </tr>
      <tr>
        <td id="L980" class="blob-num js-line-number" data-line-number="980"></td>
        <td id="LC980" class="blob-code js-file-line">        <span class="p">.</span><span class="nx">attr</span><span class="p">(</span><span class="s2">&quot;class&quot;</span><span class="p">,</span> <span class="s2">&quot;value primary&quot;</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L981" class="blob-num js-line-number" data-line-number="981"></td>
        <td id="LC981" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L982" class="blob-num js-line-number" data-line-number="982"></td>
        <td id="LC982" class="blob-code js-file-line">    <span class="nx">selection</span><span class="p">.</span><span class="nx">append</span><span class="p">(</span><span class="s2">&quot;span&quot;</span><span class="p">)</span></td>
      </tr>
      <tr>
        <td id="L983" class="blob-num js-line-number" data-line-number="983"></td>
        <td id="LC983" class="blob-code js-file-line">        <span class="p">.</span><span class="nx">attr</span><span class="p">(</span><span class="s2">&quot;class&quot;</span><span class="p">,</span> <span class="s2">&quot;value change&quot;</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L984" class="blob-num js-line-number" data-line-number="984"></td>
        <td id="LC984" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L985" class="blob-num js-line-number" data-line-number="985"></td>
        <td id="LC985" class="blob-code js-file-line">    <span class="nx">selection</span><span class="p">.</span><span class="nx">each</span><span class="p">(</span><span class="kd">function</span><span class="p">(</span><span class="nx">d</span><span class="p">,</span> <span class="nx">i</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L986" class="blob-num js-line-number" data-line-number="986"></td>
        <td id="LC986" class="blob-code js-file-line">      <span class="kd">var</span> <span class="nx">that</span> <span class="o">=</span> <span class="k">this</span><span class="p">,</span></td>
      </tr>
      <tr>
        <td id="L987" class="blob-num js-line-number" data-line-number="987"></td>
        <td id="LC987" class="blob-code js-file-line">          <span class="nx">id</span> <span class="o">=</span> <span class="o">++</span><span class="nx">cubism_id</span><span class="p">,</span></td>
      </tr>
      <tr>
        <td id="L988" class="blob-num js-line-number" data-line-number="988"></td>
        <td id="LC988" class="blob-code js-file-line">          <span class="nx">primary_</span> <span class="o">=</span> <span class="k">typeof</span> <span class="nx">primary</span> <span class="o">===</span> <span class="s2">&quot;function&quot;</span> <span class="o">?</span> <span class="nx">primary</span><span class="p">.</span><span class="nx">call</span><span class="p">(</span><span class="nx">that</span><span class="p">,</span> <span class="nx">d</span><span class="p">,</span> <span class="nx">i</span><span class="p">)</span> <span class="o">:</span> <span class="nx">primary</span><span class="p">,</span></td>
      </tr>
      <tr>
        <td id="L989" class="blob-num js-line-number" data-line-number="989"></td>
        <td id="LC989" class="blob-code js-file-line">          <span class="nx">secondary_</span> <span class="o">=</span> <span class="k">typeof</span> <span class="nx">secondary</span> <span class="o">===</span> <span class="s2">&quot;function&quot;</span> <span class="o">?</span> <span class="nx">secondary</span><span class="p">.</span><span class="nx">call</span><span class="p">(</span><span class="nx">that</span><span class="p">,</span> <span class="nx">d</span><span class="p">,</span> <span class="nx">i</span><span class="p">)</span> <span class="o">:</span> <span class="nx">secondary</span><span class="p">,</span></td>
      </tr>
      <tr>
        <td id="L990" class="blob-num js-line-number" data-line-number="990"></td>
        <td id="LC990" class="blob-code js-file-line">          <span class="nx">extent_</span> <span class="o">=</span> <span class="k">typeof</span> <span class="nx">extent</span> <span class="o">===</span> <span class="s2">&quot;function&quot;</span> <span class="o">?</span> <span class="nx">extent</span><span class="p">.</span><span class="nx">call</span><span class="p">(</span><span class="nx">that</span><span class="p">,</span> <span class="nx">d</span><span class="p">,</span> <span class="nx">i</span><span class="p">)</span> <span class="o">:</span> <span class="nx">extent</span><span class="p">,</span></td>
      </tr>
      <tr>
        <td id="L991" class="blob-num js-line-number" data-line-number="991"></td>
        <td id="LC991" class="blob-code js-file-line">          <span class="nx">div</span> <span class="o">=</span> <span class="nx">d3</span><span class="p">.</span><span class="nx">select</span><span class="p">(</span><span class="nx">that</span><span class="p">),</span></td>
      </tr>
      <tr>
        <td id="L992" class="blob-num js-line-number" data-line-number="992"></td>
        <td id="LC992" class="blob-code js-file-line">          <span class="nx">canvas</span> <span class="o">=</span> <span class="nx">div</span><span class="p">.</span><span class="nx">select</span><span class="p">(</span><span class="s2">&quot;canvas&quot;</span><span class="p">),</span></td>
      </tr>
      <tr>
        <td id="L993" class="blob-num js-line-number" data-line-number="993"></td>
        <td id="LC993" class="blob-code js-file-line">          <span class="nx">spanPrimary</span> <span class="o">=</span> <span class="nx">div</span><span class="p">.</span><span class="nx">select</span><span class="p">(</span><span class="s2">&quot;.value.primary&quot;</span><span class="p">),</span></td>
      </tr>
      <tr>
        <td id="L994" class="blob-num js-line-number" data-line-number="994"></td>
        <td id="LC994" class="blob-code js-file-line">          <span class="nx">spanChange</span> <span class="o">=</span> <span class="nx">div</span><span class="p">.</span><span class="nx">select</span><span class="p">(</span><span class="s2">&quot;.value.change&quot;</span><span class="p">),</span></td>
      </tr>
      <tr>
        <td id="L995" class="blob-num js-line-number" data-line-number="995"></td>
        <td id="LC995" class="blob-code js-file-line">          <span class="nx">ready</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L996" class="blob-num js-line-number" data-line-number="996"></td>
        <td id="LC996" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L997" class="blob-num js-line-number" data-line-number="997"></td>
        <td id="LC997" class="blob-code js-file-line">      <span class="nx">canvas</span><span class="p">.</span><span class="nx">datum</span><span class="p">({</span><span class="nx">id</span><span class="o">:</span> <span class="nx">id</span><span class="p">,</span> <span class="nx">primary</span><span class="o">:</span> <span class="nx">primary_</span><span class="p">,</span> <span class="nx">secondary</span><span class="o">:</span> <span class="nx">secondary_</span><span class="p">});</span></td>
      </tr>
      <tr>
        <td id="L998" class="blob-num js-line-number" data-line-number="998"></td>
        <td id="LC998" class="blob-code js-file-line">      <span class="nx">canvas</span> <span class="o">=</span> <span class="nx">canvas</span><span class="p">.</span><span class="nx">node</span><span class="p">().</span><span class="nx">getContext</span><span class="p">(</span><span class="s2">&quot;2d&quot;</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L999" class="blob-num js-line-number" data-line-number="999"></td>
        <td id="LC999" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L1000" class="blob-num js-line-number" data-line-number="1000"></td>
        <td id="LC1000" class="blob-code js-file-line">      <span class="kd">function</span> <span class="nx">change</span><span class="p">(</span><span class="nx">start</span><span class="p">,</span> <span class="nx">stop</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L1001" class="blob-num js-line-number" data-line-number="1001"></td>
        <td id="LC1001" class="blob-code js-file-line">        <span class="nx">canvas</span><span class="p">.</span><span class="nx">save</span><span class="p">();</span></td>
      </tr>
      <tr>
        <td id="L1002" class="blob-num js-line-number" data-line-number="1002"></td>
        <td id="LC1002" class="blob-code js-file-line">        <span class="nx">canvas</span><span class="p">.</span><span class="nx">clearRect</span><span class="p">(</span><span class="mi">0</span><span class="p">,</span> <span class="mi">0</span><span class="p">,</span> <span class="nx">width</span><span class="p">,</span> <span class="nx">height</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L1003" class="blob-num js-line-number" data-line-number="1003"></td>
        <td id="LC1003" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L1004" class="blob-num js-line-number" data-line-number="1004"></td>
        <td id="LC1004" class="blob-code js-file-line">        <span class="c1">// update the scale</span></td>
      </tr>
      <tr>
        <td id="L1005" class="blob-num js-line-number" data-line-number="1005"></td>
        <td id="LC1005" class="blob-code js-file-line">        <span class="kd">var</span> <span class="nx">primaryExtent</span> <span class="o">=</span> <span class="nx">primary_</span><span class="p">.</span><span class="nx">extent</span><span class="p">(),</span></td>
      </tr>
      <tr>
        <td id="L1006" class="blob-num js-line-number" data-line-number="1006"></td>
        <td id="LC1006" class="blob-code js-file-line">            <span class="nx">secondaryExtent</span> <span class="o">=</span> <span class="nx">secondary_</span><span class="p">.</span><span class="nx">extent</span><span class="p">(),</span></td>
      </tr>
      <tr>
        <td id="L1007" class="blob-num js-line-number" data-line-number="1007"></td>
        <td id="LC1007" class="blob-code js-file-line">            <span class="nx">extent</span> <span class="o">=</span> <span class="nx">extent_</span> <span class="o">==</span> <span class="kc">null</span> <span class="o">?</span> <span class="nx">primaryExtent</span> <span class="o">:</span> <span class="nx">extent_</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L1008" class="blob-num js-line-number" data-line-number="1008"></td>
        <td id="LC1008" class="blob-code js-file-line">        <span class="nx">scale</span><span class="p">.</span><span class="nx">domain</span><span class="p">(</span><span class="nx">extent</span><span class="p">).</span><span class="nx">range</span><span class="p">([</span><span class="nx">height</span><span class="p">,</span> <span class="mi">0</span><span class="p">]);</span></td>
      </tr>
      <tr>
        <td id="L1009" class="blob-num js-line-number" data-line-number="1009"></td>
        <td id="LC1009" class="blob-code js-file-line">        <span class="nx">ready</span> <span class="o">=</span> <span class="nx">primaryExtent</span><span class="p">.</span><span class="nx">concat</span><span class="p">(</span><span class="nx">secondaryExtent</span><span class="p">).</span><span class="nx">every</span><span class="p">(</span><span class="nb">isFinite</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L1010" class="blob-num js-line-number" data-line-number="1010"></td>
        <td id="LC1010" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L1011" class="blob-num js-line-number" data-line-number="1011"></td>
        <td id="LC1011" class="blob-code js-file-line">        <span class="c1">// consistent overplotting</span></td>
      </tr>
      <tr>
        <td id="L1012" class="blob-num js-line-number" data-line-number="1012"></td>
        <td id="LC1012" class="blob-code js-file-line">        <span class="kd">var</span> <span class="nx">round</span> <span class="o">=</span> <span class="nx">start</span> <span class="o">/</span> <span class="nx">context</span><span class="p">.</span><span class="nx">step</span><span class="p">()</span> <span class="o">&amp;</span> <span class="mi">1</span></td>
      </tr>
      <tr>
        <td id="L1013" class="blob-num js-line-number" data-line-number="1013"></td>
        <td id="LC1013" class="blob-code js-file-line">            <span class="o">?</span> <span class="nx">cubism_comparisonRoundOdd</span></td>
      </tr>
      <tr>
        <td id="L1014" class="blob-num js-line-number" data-line-number="1014"></td>
        <td id="LC1014" class="blob-code js-file-line">            <span class="o">:</span> <span class="nx">cubism_comparisonRoundEven</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L1015" class="blob-num js-line-number" data-line-number="1015"></td>
        <td id="LC1015" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L1016" class="blob-num js-line-number" data-line-number="1016"></td>
        <td id="LC1016" class="blob-code js-file-line">        <span class="c1">// positive changes</span></td>
      </tr>
      <tr>
        <td id="L1017" class="blob-num js-line-number" data-line-number="1017"></td>
        <td id="LC1017" class="blob-code js-file-line">        <span class="nx">canvas</span><span class="p">.</span><span class="nx">fillStyle</span> <span class="o">=</span> <span class="nx">colors</span><span class="p">[</span><span class="mi">2</span><span class="p">];</span></td>
      </tr>
      <tr>
        <td id="L1018" class="blob-num js-line-number" data-line-number="1018"></td>
        <td id="LC1018" class="blob-code js-file-line">        <span class="k">for</span> <span class="p">(</span><span class="kd">var</span> <span class="nx">i</span> <span class="o">=</span> <span class="mi">0</span><span class="p">,</span> <span class="nx">n</span> <span class="o">=</span> <span class="nx">width</span><span class="p">;</span> <span class="nx">i</span> <span class="o">&lt;</span> <span class="nx">n</span><span class="p">;</span> <span class="o">++</span><span class="nx">i</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L1019" class="blob-num js-line-number" data-line-number="1019"></td>
        <td id="LC1019" class="blob-code js-file-line">          <span class="kd">var</span> <span class="nx">y0</span> <span class="o">=</span> <span class="nx">scale</span><span class="p">(</span><span class="nx">primary_</span><span class="p">.</span><span class="nx">valueAt</span><span class="p">(</span><span class="nx">i</span><span class="p">)),</span></td>
      </tr>
      <tr>
        <td id="L1020" class="blob-num js-line-number" data-line-number="1020"></td>
        <td id="LC1020" class="blob-code js-file-line">              <span class="nx">y1</span> <span class="o">=</span> <span class="nx">scale</span><span class="p">(</span><span class="nx">secondary_</span><span class="p">.</span><span class="nx">valueAt</span><span class="p">(</span><span class="nx">i</span><span class="p">));</span></td>
      </tr>
      <tr>
        <td id="L1021" class="blob-num js-line-number" data-line-number="1021"></td>
        <td id="LC1021" class="blob-code js-file-line">          <span class="k">if</span> <span class="p">(</span><span class="nx">y0</span> <span class="o">&lt;</span> <span class="nx">y1</span><span class="p">)</span> <span class="nx">canvas</span><span class="p">.</span><span class="nx">fillRect</span><span class="p">(</span><span class="nx">round</span><span class="p">(</span><span class="nx">i</span><span class="p">),</span> <span class="nx">y0</span><span class="p">,</span> <span class="mi">1</span><span class="p">,</span> <span class="nx">y1</span> <span class="o">-</span> <span class="nx">y0</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L1022" class="blob-num js-line-number" data-line-number="1022"></td>
        <td id="LC1022" class="blob-code js-file-line">        <span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L1023" class="blob-num js-line-number" data-line-number="1023"></td>
        <td id="LC1023" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L1024" class="blob-num js-line-number" data-line-number="1024"></td>
        <td id="LC1024" class="blob-code js-file-line">        <span class="c1">// negative changes</span></td>
      </tr>
      <tr>
        <td id="L1025" class="blob-num js-line-number" data-line-number="1025"></td>
        <td id="LC1025" class="blob-code js-file-line">        <span class="nx">canvas</span><span class="p">.</span><span class="nx">fillStyle</span> <span class="o">=</span> <span class="nx">colors</span><span class="p">[</span><span class="mi">0</span><span class="p">];</span></td>
      </tr>
      <tr>
        <td id="L1026" class="blob-num js-line-number" data-line-number="1026"></td>
        <td id="LC1026" class="blob-code js-file-line">        <span class="k">for</span> <span class="p">(</span><span class="nx">i</span> <span class="o">=</span> <span class="mi">0</span><span class="p">;</span> <span class="nx">i</span> <span class="o">&lt;</span> <span class="nx">n</span><span class="p">;</span> <span class="o">++</span><span class="nx">i</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L1027" class="blob-num js-line-number" data-line-number="1027"></td>
        <td id="LC1027" class="blob-code js-file-line">          <span class="kd">var</span> <span class="nx">y0</span> <span class="o">=</span> <span class="nx">scale</span><span class="p">(</span><span class="nx">primary_</span><span class="p">.</span><span class="nx">valueAt</span><span class="p">(</span><span class="nx">i</span><span class="p">)),</span></td>
      </tr>
      <tr>
        <td id="L1028" class="blob-num js-line-number" data-line-number="1028"></td>
        <td id="LC1028" class="blob-code js-file-line">              <span class="nx">y1</span> <span class="o">=</span> <span class="nx">scale</span><span class="p">(</span><span class="nx">secondary_</span><span class="p">.</span><span class="nx">valueAt</span><span class="p">(</span><span class="nx">i</span><span class="p">));</span></td>
      </tr>
      <tr>
        <td id="L1029" class="blob-num js-line-number" data-line-number="1029"></td>
        <td id="LC1029" class="blob-code js-file-line">          <span class="k">if</span> <span class="p">(</span><span class="nx">y0</span> <span class="o">&gt;</span> <span class="nx">y1</span><span class="p">)</span> <span class="nx">canvas</span><span class="p">.</span><span class="nx">fillRect</span><span class="p">(</span><span class="nx">round</span><span class="p">(</span><span class="nx">i</span><span class="p">),</span> <span class="nx">y1</span><span class="p">,</span> <span class="mi">1</span><span class="p">,</span> <span class="nx">y0</span> <span class="o">-</span> <span class="nx">y1</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L1030" class="blob-num js-line-number" data-line-number="1030"></td>
        <td id="LC1030" class="blob-code js-file-line">        <span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L1031" class="blob-num js-line-number" data-line-number="1031"></td>
        <td id="LC1031" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L1032" class="blob-num js-line-number" data-line-number="1032"></td>
        <td id="LC1032" class="blob-code js-file-line">        <span class="c1">// positive values</span></td>
      </tr>
      <tr>
        <td id="L1033" class="blob-num js-line-number" data-line-number="1033"></td>
        <td id="LC1033" class="blob-code js-file-line">        <span class="nx">canvas</span><span class="p">.</span><span class="nx">fillStyle</span> <span class="o">=</span> <span class="nx">colors</span><span class="p">[</span><span class="mi">3</span><span class="p">];</span></td>
      </tr>
      <tr>
        <td id="L1034" class="blob-num js-line-number" data-line-number="1034"></td>
        <td id="LC1034" class="blob-code js-file-line">        <span class="k">for</span> <span class="p">(</span><span class="nx">i</span> <span class="o">=</span> <span class="mi">0</span><span class="p">;</span> <span class="nx">i</span> <span class="o">&lt;</span> <span class="nx">n</span><span class="p">;</span> <span class="o">++</span><span class="nx">i</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L1035" class="blob-num js-line-number" data-line-number="1035"></td>
        <td id="LC1035" class="blob-code js-file-line">          <span class="kd">var</span> <span class="nx">y0</span> <span class="o">=</span> <span class="nx">scale</span><span class="p">(</span><span class="nx">primary_</span><span class="p">.</span><span class="nx">valueAt</span><span class="p">(</span><span class="nx">i</span><span class="p">)),</span></td>
      </tr>
      <tr>
        <td id="L1036" class="blob-num js-line-number" data-line-number="1036"></td>
        <td id="LC1036" class="blob-code js-file-line">              <span class="nx">y1</span> <span class="o">=</span> <span class="nx">scale</span><span class="p">(</span><span class="nx">secondary_</span><span class="p">.</span><span class="nx">valueAt</span><span class="p">(</span><span class="nx">i</span><span class="p">));</span></td>
      </tr>
      <tr>
        <td id="L1037" class="blob-num js-line-number" data-line-number="1037"></td>
        <td id="LC1037" class="blob-code js-file-line">          <span class="k">if</span> <span class="p">(</span><span class="nx">y0</span> <span class="o">&lt;=</span> <span class="nx">y1</span><span class="p">)</span> <span class="nx">canvas</span><span class="p">.</span><span class="nx">fillRect</span><span class="p">(</span><span class="nx">round</span><span class="p">(</span><span class="nx">i</span><span class="p">),</span> <span class="nx">y0</span><span class="p">,</span> <span class="mi">1</span><span class="p">,</span> <span class="nx">strokeWidth</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L1038" class="blob-num js-line-number" data-line-number="1038"></td>
        <td id="LC1038" class="blob-code js-file-line">        <span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L1039" class="blob-num js-line-number" data-line-number="1039"></td>
        <td id="LC1039" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L1040" class="blob-num js-line-number" data-line-number="1040"></td>
        <td id="LC1040" class="blob-code js-file-line">        <span class="c1">// negative values</span></td>
      </tr>
      <tr>
        <td id="L1041" class="blob-num js-line-number" data-line-number="1041"></td>
        <td id="LC1041" class="blob-code js-file-line">        <span class="nx">canvas</span><span class="p">.</span><span class="nx">fillStyle</span> <span class="o">=</span> <span class="nx">colors</span><span class="p">[</span><span class="mi">1</span><span class="p">];</span></td>
      </tr>
      <tr>
        <td id="L1042" class="blob-num js-line-number" data-line-number="1042"></td>
        <td id="LC1042" class="blob-code js-file-line">        <span class="k">for</span> <span class="p">(</span><span class="nx">i</span> <span class="o">=</span> <span class="mi">0</span><span class="p">;</span> <span class="nx">i</span> <span class="o">&lt;</span> <span class="nx">n</span><span class="p">;</span> <span class="o">++</span><span class="nx">i</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L1043" class="blob-num js-line-number" data-line-number="1043"></td>
        <td id="LC1043" class="blob-code js-file-line">          <span class="kd">var</span> <span class="nx">y0</span> <span class="o">=</span> <span class="nx">scale</span><span class="p">(</span><span class="nx">primary_</span><span class="p">.</span><span class="nx">valueAt</span><span class="p">(</span><span class="nx">i</span><span class="p">)),</span></td>
      </tr>
      <tr>
        <td id="L1044" class="blob-num js-line-number" data-line-number="1044"></td>
        <td id="LC1044" class="blob-code js-file-line">              <span class="nx">y1</span> <span class="o">=</span> <span class="nx">scale</span><span class="p">(</span><span class="nx">secondary_</span><span class="p">.</span><span class="nx">valueAt</span><span class="p">(</span><span class="nx">i</span><span class="p">));</span></td>
      </tr>
      <tr>
        <td id="L1045" class="blob-num js-line-number" data-line-number="1045"></td>
        <td id="LC1045" class="blob-code js-file-line">          <span class="k">if</span> <span class="p">(</span><span class="nx">y0</span> <span class="o">&gt;</span> <span class="nx">y1</span><span class="p">)</span> <span class="nx">canvas</span><span class="p">.</span><span class="nx">fillRect</span><span class="p">(</span><span class="nx">round</span><span class="p">(</span><span class="nx">i</span><span class="p">),</span> <span class="nx">y0</span> <span class="o">-</span> <span class="nx">strokeWidth</span><span class="p">,</span> <span class="mi">1</span><span class="p">,</span> <span class="nx">strokeWidth</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L1046" class="blob-num js-line-number" data-line-number="1046"></td>
        <td id="LC1046" class="blob-code js-file-line">        <span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L1047" class="blob-num js-line-number" data-line-number="1047"></td>
        <td id="LC1047" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L1048" class="blob-num js-line-number" data-line-number="1048"></td>
        <td id="LC1048" class="blob-code js-file-line">        <span class="nx">canvas</span><span class="p">.</span><span class="nx">restore</span><span class="p">();</span></td>
      </tr>
      <tr>
        <td id="L1049" class="blob-num js-line-number" data-line-number="1049"></td>
        <td id="LC1049" class="blob-code js-file-line">      <span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L1050" class="blob-num js-line-number" data-line-number="1050"></td>
        <td id="LC1050" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L1051" class="blob-num js-line-number" data-line-number="1051"></td>
        <td id="LC1051" class="blob-code js-file-line">      <span class="kd">function</span> <span class="nx">focus</span><span class="p">(</span><span class="nx">i</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L1052" class="blob-num js-line-number" data-line-number="1052"></td>
        <td id="LC1052" class="blob-code js-file-line">        <span class="k">if</span> <span class="p">(</span><span class="nx">i</span> <span class="o">==</span> <span class="kc">null</span><span class="p">)</span> <span class="nx">i</span> <span class="o">=</span> <span class="nx">width</span> <span class="o">-</span> <span class="mi">1</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L1053" class="blob-num js-line-number" data-line-number="1053"></td>
        <td id="LC1053" class="blob-code js-file-line">        <span class="kd">var</span> <span class="nx">valuePrimary</span> <span class="o">=</span> <span class="nx">primary_</span><span class="p">.</span><span class="nx">valueAt</span><span class="p">(</span><span class="nx">i</span><span class="p">),</span></td>
      </tr>
      <tr>
        <td id="L1054" class="blob-num js-line-number" data-line-number="1054"></td>
        <td id="LC1054" class="blob-code js-file-line">            <span class="nx">valueSecondary</span> <span class="o">=</span> <span class="nx">secondary_</span><span class="p">.</span><span class="nx">valueAt</span><span class="p">(</span><span class="nx">i</span><span class="p">),</span></td>
      </tr>
      <tr>
        <td id="L1055" class="blob-num js-line-number" data-line-number="1055"></td>
        <td id="LC1055" class="blob-code js-file-line">            <span class="nx">valueChange</span> <span class="o">=</span> <span class="p">(</span><span class="nx">valuePrimary</span> <span class="o">-</span> <span class="nx">valueSecondary</span><span class="p">)</span> <span class="o">/</span> <span class="nx">valueSecondary</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L1056" class="blob-num js-line-number" data-line-number="1056"></td>
        <td id="LC1056" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L1057" class="blob-num js-line-number" data-line-number="1057"></td>
        <td id="LC1057" class="blob-code js-file-line">        <span class="nx">spanPrimary</span></td>
      </tr>
      <tr>
        <td id="L1058" class="blob-num js-line-number" data-line-number="1058"></td>
        <td id="LC1058" class="blob-code js-file-line">            <span class="p">.</span><span class="nx">datum</span><span class="p">(</span><span class="nx">valuePrimary</span><span class="p">)</span></td>
      </tr>
      <tr>
        <td id="L1059" class="blob-num js-line-number" data-line-number="1059"></td>
        <td id="LC1059" class="blob-code js-file-line">            <span class="p">.</span><span class="nx">text</span><span class="p">(</span><span class="nb">isNaN</span><span class="p">(</span><span class="nx">valuePrimary</span><span class="p">)</span> <span class="o">?</span> <span class="kc">null</span> <span class="o">:</span> <span class="nx">formatPrimary</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L1060" class="blob-num js-line-number" data-line-number="1060"></td>
        <td id="LC1060" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L1061" class="blob-num js-line-number" data-line-number="1061"></td>
        <td id="LC1061" class="blob-code js-file-line">        <span class="nx">spanChange</span></td>
      </tr>
      <tr>
        <td id="L1062" class="blob-num js-line-number" data-line-number="1062"></td>
        <td id="LC1062" class="blob-code js-file-line">            <span class="p">.</span><span class="nx">datum</span><span class="p">(</span><span class="nx">valueChange</span><span class="p">)</span></td>
      </tr>
      <tr>
        <td id="L1063" class="blob-num js-line-number" data-line-number="1063"></td>
        <td id="LC1063" class="blob-code js-file-line">            <span class="p">.</span><span class="nx">text</span><span class="p">(</span><span class="nb">isNaN</span><span class="p">(</span><span class="nx">valueChange</span><span class="p">)</span> <span class="o">?</span> <span class="kc">null</span> <span class="o">:</span> <span class="nx">formatChange</span><span class="p">)</span></td>
      </tr>
      <tr>
        <td id="L1064" class="blob-num js-line-number" data-line-number="1064"></td>
        <td id="LC1064" class="blob-code js-file-line">            <span class="p">.</span><span class="nx">attr</span><span class="p">(</span><span class="s2">&quot;class&quot;</span><span class="p">,</span> <span class="s2">&quot;value change &quot;</span> <span class="o">+</span> <span class="p">(</span><span class="nx">valueChange</span> <span class="o">&gt;</span> <span class="mi">0</span> <span class="o">?</span> <span class="s2">&quot;positive&quot;</span> <span class="o">:</span> <span class="nx">valueChange</span> <span class="o">&lt;</span> <span class="mi">0</span> <span class="o">?</span> <span class="s2">&quot;negative&quot;</span> <span class="o">:</span> <span class="s2">&quot;&quot;</span><span class="p">));</span></td>
      </tr>
      <tr>
        <td id="L1065" class="blob-num js-line-number" data-line-number="1065"></td>
        <td id="LC1065" class="blob-code js-file-line">      <span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L1066" class="blob-num js-line-number" data-line-number="1066"></td>
        <td id="LC1066" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L1067" class="blob-num js-line-number" data-line-number="1067"></td>
        <td id="LC1067" class="blob-code js-file-line">      <span class="c1">// Display the first primary change immediately,</span></td>
      </tr>
      <tr>
        <td id="L1068" class="blob-num js-line-number" data-line-number="1068"></td>
        <td id="LC1068" class="blob-code js-file-line">      <span class="c1">// but defer subsequent updates to the context change.</span></td>
      </tr>
      <tr>
        <td id="L1069" class="blob-num js-line-number" data-line-number="1069"></td>
        <td id="LC1069" class="blob-code js-file-line">      <span class="c1">// Note that someone still needs to listen to the metric,</span></td>
      </tr>
      <tr>
        <td id="L1070" class="blob-num js-line-number" data-line-number="1070"></td>
        <td id="LC1070" class="blob-code js-file-line">      <span class="c1">// so that it continues to update automatically.</span></td>
      </tr>
      <tr>
        <td id="L1071" class="blob-num js-line-number" data-line-number="1071"></td>
        <td id="LC1071" class="blob-code js-file-line">      <span class="nx">primary_</span><span class="p">.</span><span class="nx">on</span><span class="p">(</span><span class="s2">&quot;change.comparison-&quot;</span> <span class="o">+</span> <span class="nx">id</span><span class="p">,</span> <span class="nx">firstChange</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L1072" class="blob-num js-line-number" data-line-number="1072"></td>
        <td id="LC1072" class="blob-code js-file-line">      <span class="nx">secondary_</span><span class="p">.</span><span class="nx">on</span><span class="p">(</span><span class="s2">&quot;change.comparison-&quot;</span> <span class="o">+</span> <span class="nx">id</span><span class="p">,</span> <span class="nx">firstChange</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L1073" class="blob-num js-line-number" data-line-number="1073"></td>
        <td id="LC1073" class="blob-code js-file-line">      <span class="kd">function</span> <span class="nx">firstChange</span><span class="p">(</span><span class="nx">start</span><span class="p">,</span> <span class="nx">stop</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L1074" class="blob-num js-line-number" data-line-number="1074"></td>
        <td id="LC1074" class="blob-code js-file-line">        <span class="nx">change</span><span class="p">(</span><span class="nx">start</span><span class="p">,</span> <span class="nx">stop</span><span class="p">),</span> <span class="nx">focus</span><span class="p">();</span></td>
      </tr>
      <tr>
        <td id="L1075" class="blob-num js-line-number" data-line-number="1075"></td>
        <td id="LC1075" class="blob-code js-file-line">        <span class="k">if</span> <span class="p">(</span><span class="nx">ready</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L1076" class="blob-num js-line-number" data-line-number="1076"></td>
        <td id="LC1076" class="blob-code js-file-line">          <span class="nx">primary_</span><span class="p">.</span><span class="nx">on</span><span class="p">(</span><span class="s2">&quot;change.comparison-&quot;</span> <span class="o">+</span> <span class="nx">id</span><span class="p">,</span> <span class="nx">cubism_identity</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L1077" class="blob-num js-line-number" data-line-number="1077"></td>
        <td id="LC1077" class="blob-code js-file-line">          <span class="nx">secondary_</span><span class="p">.</span><span class="nx">on</span><span class="p">(</span><span class="s2">&quot;change.comparison-&quot;</span> <span class="o">+</span> <span class="nx">id</span><span class="p">,</span> <span class="nx">cubism_identity</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L1078" class="blob-num js-line-number" data-line-number="1078"></td>
        <td id="LC1078" class="blob-code js-file-line">        <span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L1079" class="blob-num js-line-number" data-line-number="1079"></td>
        <td id="LC1079" class="blob-code js-file-line">      <span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L1080" class="blob-num js-line-number" data-line-number="1080"></td>
        <td id="LC1080" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L1081" class="blob-num js-line-number" data-line-number="1081"></td>
        <td id="LC1081" class="blob-code js-file-line">      <span class="c1">// Update the chart when the context changes.</span></td>
      </tr>
      <tr>
        <td id="L1082" class="blob-num js-line-number" data-line-number="1082"></td>
        <td id="LC1082" class="blob-code js-file-line">      <span class="nx">context</span><span class="p">.</span><span class="nx">on</span><span class="p">(</span><span class="s2">&quot;change.comparison-&quot;</span> <span class="o">+</span> <span class="nx">id</span><span class="p">,</span> <span class="nx">change</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L1083" class="blob-num js-line-number" data-line-number="1083"></td>
        <td id="LC1083" class="blob-code js-file-line">      <span class="nx">context</span><span class="p">.</span><span class="nx">on</span><span class="p">(</span><span class="s2">&quot;focus.comparison-&quot;</span> <span class="o">+</span> <span class="nx">id</span><span class="p">,</span> <span class="nx">focus</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L1084" class="blob-num js-line-number" data-line-number="1084"></td>
        <td id="LC1084" class="blob-code js-file-line">    <span class="p">});</span></td>
      </tr>
      <tr>
        <td id="L1085" class="blob-num js-line-number" data-line-number="1085"></td>
        <td id="LC1085" class="blob-code js-file-line">  <span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L1086" class="blob-num js-line-number" data-line-number="1086"></td>
        <td id="LC1086" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L1087" class="blob-num js-line-number" data-line-number="1087"></td>
        <td id="LC1087" class="blob-code js-file-line">  <span class="nx">comparison</span><span class="p">.</span><span class="nx">remove</span> <span class="o">=</span> <span class="kd">function</span><span class="p">(</span><span class="nx">selection</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L1088" class="blob-num js-line-number" data-line-number="1088"></td>
        <td id="LC1088" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L1089" class="blob-num js-line-number" data-line-number="1089"></td>
        <td id="LC1089" class="blob-code js-file-line">    <span class="nx">selection</span></td>
      </tr>
      <tr>
        <td id="L1090" class="blob-num js-line-number" data-line-number="1090"></td>
        <td id="LC1090" class="blob-code js-file-line">        <span class="p">.</span><span class="nx">on</span><span class="p">(</span><span class="s2">&quot;mousemove.comparison&quot;</span><span class="p">,</span> <span class="kc">null</span><span class="p">)</span></td>
      </tr>
      <tr>
        <td id="L1091" class="blob-num js-line-number" data-line-number="1091"></td>
        <td id="LC1091" class="blob-code js-file-line">        <span class="p">.</span><span class="nx">on</span><span class="p">(</span><span class="s2">&quot;mouseout.comparison&quot;</span><span class="p">,</span> <span class="kc">null</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L1092" class="blob-num js-line-number" data-line-number="1092"></td>
        <td id="LC1092" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L1093" class="blob-num js-line-number" data-line-number="1093"></td>
        <td id="LC1093" class="blob-code js-file-line">    <span class="nx">selection</span><span class="p">.</span><span class="nx">selectAll</span><span class="p">(</span><span class="s2">&quot;canvas&quot;</span><span class="p">)</span></td>
      </tr>
      <tr>
        <td id="L1094" class="blob-num js-line-number" data-line-number="1094"></td>
        <td id="LC1094" class="blob-code js-file-line">        <span class="p">.</span><span class="nx">each</span><span class="p">(</span><span class="nx">remove</span><span class="p">)</span></td>
      </tr>
      <tr>
        <td id="L1095" class="blob-num js-line-number" data-line-number="1095"></td>
        <td id="LC1095" class="blob-code js-file-line">        <span class="p">.</span><span class="nx">remove</span><span class="p">();</span></td>
      </tr>
      <tr>
        <td id="L1096" class="blob-num js-line-number" data-line-number="1096"></td>
        <td id="LC1096" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L1097" class="blob-num js-line-number" data-line-number="1097"></td>
        <td id="LC1097" class="blob-code js-file-line">    <span class="nx">selection</span><span class="p">.</span><span class="nx">selectAll</span><span class="p">(</span><span class="s2">&quot;.title,.value&quot;</span><span class="p">)</span></td>
      </tr>
      <tr>
        <td id="L1098" class="blob-num js-line-number" data-line-number="1098"></td>
        <td id="LC1098" class="blob-code js-file-line">        <span class="p">.</span><span class="nx">remove</span><span class="p">();</span></td>
      </tr>
      <tr>
        <td id="L1099" class="blob-num js-line-number" data-line-number="1099"></td>
        <td id="LC1099" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L1100" class="blob-num js-line-number" data-line-number="1100"></td>
        <td id="LC1100" class="blob-code js-file-line">    <span class="kd">function</span> <span class="nx">remove</span><span class="p">(</span><span class="nx">d</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L1101" class="blob-num js-line-number" data-line-number="1101"></td>
        <td id="LC1101" class="blob-code js-file-line">      <span class="nx">d</span><span class="p">.</span><span class="nx">primary</span><span class="p">.</span><span class="nx">on</span><span class="p">(</span><span class="s2">&quot;change.comparison-&quot;</span> <span class="o">+</span> <span class="nx">d</span><span class="p">.</span><span class="nx">id</span><span class="p">,</span> <span class="kc">null</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L1102" class="blob-num js-line-number" data-line-number="1102"></td>
        <td id="LC1102" class="blob-code js-file-line">      <span class="nx">d</span><span class="p">.</span><span class="nx">secondary</span><span class="p">.</span><span class="nx">on</span><span class="p">(</span><span class="s2">&quot;change.comparison-&quot;</span> <span class="o">+</span> <span class="nx">d</span><span class="p">.</span><span class="nx">id</span><span class="p">,</span> <span class="kc">null</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L1103" class="blob-num js-line-number" data-line-number="1103"></td>
        <td id="LC1103" class="blob-code js-file-line">      <span class="nx">context</span><span class="p">.</span><span class="nx">on</span><span class="p">(</span><span class="s2">&quot;change.comparison-&quot;</span> <span class="o">+</span> <span class="nx">d</span><span class="p">.</span><span class="nx">id</span><span class="p">,</span> <span class="kc">null</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L1104" class="blob-num js-line-number" data-line-number="1104"></td>
        <td id="LC1104" class="blob-code js-file-line">      <span class="nx">context</span><span class="p">.</span><span class="nx">on</span><span class="p">(</span><span class="s2">&quot;focus.comparison-&quot;</span> <span class="o">+</span> <span class="nx">d</span><span class="p">.</span><span class="nx">id</span><span class="p">,</span> <span class="kc">null</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L1105" class="blob-num js-line-number" data-line-number="1105"></td>
        <td id="LC1105" class="blob-code js-file-line">    <span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L1106" class="blob-num js-line-number" data-line-number="1106"></td>
        <td id="LC1106" class="blob-code js-file-line">  <span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L1107" class="blob-num js-line-number" data-line-number="1107"></td>
        <td id="LC1107" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L1108" class="blob-num js-line-number" data-line-number="1108"></td>
        <td id="LC1108" class="blob-code js-file-line">  <span class="nx">comparison</span><span class="p">.</span><span class="nx">height</span> <span class="o">=</span> <span class="kd">function</span><span class="p">(</span><span class="nx">_</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L1109" class="blob-num js-line-number" data-line-number="1109"></td>
        <td id="LC1109" class="blob-code js-file-line">    <span class="k">if</span> <span class="p">(</span><span class="o">!</span><span class="nx">arguments</span><span class="p">.</span><span class="nx">length</span><span class="p">)</span> <span class="k">return</span> <span class="nx">height</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L1110" class="blob-num js-line-number" data-line-number="1110"></td>
        <td id="LC1110" class="blob-code js-file-line">    <span class="nx">height</span> <span class="o">=</span> <span class="o">+</span><span class="nx">_</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L1111" class="blob-num js-line-number" data-line-number="1111"></td>
        <td id="LC1111" class="blob-code js-file-line">    <span class="k">return</span> <span class="nx">comparison</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L1112" class="blob-num js-line-number" data-line-number="1112"></td>
        <td id="LC1112" class="blob-code js-file-line">  <span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L1113" class="blob-num js-line-number" data-line-number="1113"></td>
        <td id="LC1113" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L1114" class="blob-num js-line-number" data-line-number="1114"></td>
        <td id="LC1114" class="blob-code js-file-line">  <span class="nx">comparison</span><span class="p">.</span><span class="nx">primary</span> <span class="o">=</span> <span class="kd">function</span><span class="p">(</span><span class="nx">_</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L1115" class="blob-num js-line-number" data-line-number="1115"></td>
        <td id="LC1115" class="blob-code js-file-line">    <span class="k">if</span> <span class="p">(</span><span class="o">!</span><span class="nx">arguments</span><span class="p">.</span><span class="nx">length</span><span class="p">)</span> <span class="k">return</span> <span class="nx">primary</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L1116" class="blob-num js-line-number" data-line-number="1116"></td>
        <td id="LC1116" class="blob-code js-file-line">    <span class="nx">primary</span> <span class="o">=</span> <span class="nx">_</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L1117" class="blob-num js-line-number" data-line-number="1117"></td>
        <td id="LC1117" class="blob-code js-file-line">    <span class="k">return</span> <span class="nx">comparison</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L1118" class="blob-num js-line-number" data-line-number="1118"></td>
        <td id="LC1118" class="blob-code js-file-line">  <span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L1119" class="blob-num js-line-number" data-line-number="1119"></td>
        <td id="LC1119" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L1120" class="blob-num js-line-number" data-line-number="1120"></td>
        <td id="LC1120" class="blob-code js-file-line">  <span class="nx">comparison</span><span class="p">.</span><span class="nx">secondary</span> <span class="o">=</span> <span class="kd">function</span><span class="p">(</span><span class="nx">_</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L1121" class="blob-num js-line-number" data-line-number="1121"></td>
        <td id="LC1121" class="blob-code js-file-line">    <span class="k">if</span> <span class="p">(</span><span class="o">!</span><span class="nx">arguments</span><span class="p">.</span><span class="nx">length</span><span class="p">)</span> <span class="k">return</span> <span class="nx">secondary</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L1122" class="blob-num js-line-number" data-line-number="1122"></td>
        <td id="LC1122" class="blob-code js-file-line">    <span class="nx">secondary</span> <span class="o">=</span> <span class="nx">_</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L1123" class="blob-num js-line-number" data-line-number="1123"></td>
        <td id="LC1123" class="blob-code js-file-line">    <span class="k">return</span> <span class="nx">comparison</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L1124" class="blob-num js-line-number" data-line-number="1124"></td>
        <td id="LC1124" class="blob-code js-file-line">  <span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L1125" class="blob-num js-line-number" data-line-number="1125"></td>
        <td id="LC1125" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L1126" class="blob-num js-line-number" data-line-number="1126"></td>
        <td id="LC1126" class="blob-code js-file-line">  <span class="nx">comparison</span><span class="p">.</span><span class="nx">scale</span> <span class="o">=</span> <span class="kd">function</span><span class="p">(</span><span class="nx">_</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L1127" class="blob-num js-line-number" data-line-number="1127"></td>
        <td id="LC1127" class="blob-code js-file-line">    <span class="k">if</span> <span class="p">(</span><span class="o">!</span><span class="nx">arguments</span><span class="p">.</span><span class="nx">length</span><span class="p">)</span> <span class="k">return</span> <span class="nx">scale</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L1128" class="blob-num js-line-number" data-line-number="1128"></td>
        <td id="LC1128" class="blob-code js-file-line">    <span class="nx">scale</span> <span class="o">=</span> <span class="nx">_</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L1129" class="blob-num js-line-number" data-line-number="1129"></td>
        <td id="LC1129" class="blob-code js-file-line">    <span class="k">return</span> <span class="nx">comparison</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L1130" class="blob-num js-line-number" data-line-number="1130"></td>
        <td id="LC1130" class="blob-code js-file-line">  <span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L1131" class="blob-num js-line-number" data-line-number="1131"></td>
        <td id="LC1131" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L1132" class="blob-num js-line-number" data-line-number="1132"></td>
        <td id="LC1132" class="blob-code js-file-line">  <span class="nx">comparison</span><span class="p">.</span><span class="nx">extent</span> <span class="o">=</span> <span class="kd">function</span><span class="p">(</span><span class="nx">_</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L1133" class="blob-num js-line-number" data-line-number="1133"></td>
        <td id="LC1133" class="blob-code js-file-line">    <span class="k">if</span> <span class="p">(</span><span class="o">!</span><span class="nx">arguments</span><span class="p">.</span><span class="nx">length</span><span class="p">)</span> <span class="k">return</span> <span class="nx">extent</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L1134" class="blob-num js-line-number" data-line-number="1134"></td>
        <td id="LC1134" class="blob-code js-file-line">    <span class="nx">extent</span> <span class="o">=</span> <span class="nx">_</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L1135" class="blob-num js-line-number" data-line-number="1135"></td>
        <td id="LC1135" class="blob-code js-file-line">    <span class="k">return</span> <span class="nx">comparison</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L1136" class="blob-num js-line-number" data-line-number="1136"></td>
        <td id="LC1136" class="blob-code js-file-line">  <span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L1137" class="blob-num js-line-number" data-line-number="1137"></td>
        <td id="LC1137" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L1138" class="blob-num js-line-number" data-line-number="1138"></td>
        <td id="LC1138" class="blob-code js-file-line">  <span class="nx">comparison</span><span class="p">.</span><span class="nx">title</span> <span class="o">=</span> <span class="kd">function</span><span class="p">(</span><span class="nx">_</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L1139" class="blob-num js-line-number" data-line-number="1139"></td>
        <td id="LC1139" class="blob-code js-file-line">    <span class="k">if</span> <span class="p">(</span><span class="o">!</span><span class="nx">arguments</span><span class="p">.</span><span class="nx">length</span><span class="p">)</span> <span class="k">return</span> <span class="nx">title</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L1140" class="blob-num js-line-number" data-line-number="1140"></td>
        <td id="LC1140" class="blob-code js-file-line">    <span class="nx">title</span> <span class="o">=</span> <span class="nx">_</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L1141" class="blob-num js-line-number" data-line-number="1141"></td>
        <td id="LC1141" class="blob-code js-file-line">    <span class="k">return</span> <span class="nx">comparison</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L1142" class="blob-num js-line-number" data-line-number="1142"></td>
        <td id="LC1142" class="blob-code js-file-line">  <span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L1143" class="blob-num js-line-number" data-line-number="1143"></td>
        <td id="LC1143" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L1144" class="blob-num js-line-number" data-line-number="1144"></td>
        <td id="LC1144" class="blob-code js-file-line">  <span class="nx">comparison</span><span class="p">.</span><span class="nx">formatPrimary</span> <span class="o">=</span> <span class="kd">function</span><span class="p">(</span><span class="nx">_</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L1145" class="blob-num js-line-number" data-line-number="1145"></td>
        <td id="LC1145" class="blob-code js-file-line">    <span class="k">if</span> <span class="p">(</span><span class="o">!</span><span class="nx">arguments</span><span class="p">.</span><span class="nx">length</span><span class="p">)</span> <span class="k">return</span> <span class="nx">formatPrimary</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L1146" class="blob-num js-line-number" data-line-number="1146"></td>
        <td id="LC1146" class="blob-code js-file-line">    <span class="nx">formatPrimary</span> <span class="o">=</span> <span class="nx">_</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L1147" class="blob-num js-line-number" data-line-number="1147"></td>
        <td id="LC1147" class="blob-code js-file-line">    <span class="k">return</span> <span class="nx">comparison</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L1148" class="blob-num js-line-number" data-line-number="1148"></td>
        <td id="LC1148" class="blob-code js-file-line">  <span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L1149" class="blob-num js-line-number" data-line-number="1149"></td>
        <td id="LC1149" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L1150" class="blob-num js-line-number" data-line-number="1150"></td>
        <td id="LC1150" class="blob-code js-file-line">  <span class="nx">comparison</span><span class="p">.</span><span class="nx">formatChange</span> <span class="o">=</span> <span class="kd">function</span><span class="p">(</span><span class="nx">_</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L1151" class="blob-num js-line-number" data-line-number="1151"></td>
        <td id="LC1151" class="blob-code js-file-line">    <span class="k">if</span> <span class="p">(</span><span class="o">!</span><span class="nx">arguments</span><span class="p">.</span><span class="nx">length</span><span class="p">)</span> <span class="k">return</span> <span class="nx">formatChange</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L1152" class="blob-num js-line-number" data-line-number="1152"></td>
        <td id="LC1152" class="blob-code js-file-line">    <span class="nx">formatChange</span> <span class="o">=</span> <span class="nx">_</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L1153" class="blob-num js-line-number" data-line-number="1153"></td>
        <td id="LC1153" class="blob-code js-file-line">    <span class="k">return</span> <span class="nx">comparison</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L1154" class="blob-num js-line-number" data-line-number="1154"></td>
        <td id="LC1154" class="blob-code js-file-line">  <span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L1155" class="blob-num js-line-number" data-line-number="1155"></td>
        <td id="LC1155" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L1156" class="blob-num js-line-number" data-line-number="1156"></td>
        <td id="LC1156" class="blob-code js-file-line">  <span class="nx">comparison</span><span class="p">.</span><span class="nx">colors</span> <span class="o">=</span> <span class="kd">function</span><span class="p">(</span><span class="nx">_</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L1157" class="blob-num js-line-number" data-line-number="1157"></td>
        <td id="LC1157" class="blob-code js-file-line">    <span class="k">if</span> <span class="p">(</span><span class="o">!</span><span class="nx">arguments</span><span class="p">.</span><span class="nx">length</span><span class="p">)</span> <span class="k">return</span> <span class="nx">colors</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L1158" class="blob-num js-line-number" data-line-number="1158"></td>
        <td id="LC1158" class="blob-code js-file-line">    <span class="nx">colors</span> <span class="o">=</span> <span class="nx">_</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L1159" class="blob-num js-line-number" data-line-number="1159"></td>
        <td id="LC1159" class="blob-code js-file-line">    <span class="k">return</span> <span class="nx">comparison</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L1160" class="blob-num js-line-number" data-line-number="1160"></td>
        <td id="LC1160" class="blob-code js-file-line">  <span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L1161" class="blob-num js-line-number" data-line-number="1161"></td>
        <td id="LC1161" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L1162" class="blob-num js-line-number" data-line-number="1162"></td>
        <td id="LC1162" class="blob-code js-file-line">  <span class="nx">comparison</span><span class="p">.</span><span class="nx">strokeWidth</span> <span class="o">=</span> <span class="kd">function</span><span class="p">(</span><span class="nx">_</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L1163" class="blob-num js-line-number" data-line-number="1163"></td>
        <td id="LC1163" class="blob-code js-file-line">    <span class="k">if</span> <span class="p">(</span><span class="o">!</span><span class="nx">arguments</span><span class="p">.</span><span class="nx">length</span><span class="p">)</span> <span class="k">return</span> <span class="nx">strokeWidth</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L1164" class="blob-num js-line-number" data-line-number="1164"></td>
        <td id="LC1164" class="blob-code js-file-line">    <span class="nx">strokeWidth</span> <span class="o">=</span> <span class="nx">_</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L1165" class="blob-num js-line-number" data-line-number="1165"></td>
        <td id="LC1165" class="blob-code js-file-line">    <span class="k">return</span> <span class="nx">comparison</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L1166" class="blob-num js-line-number" data-line-number="1166"></td>
        <td id="LC1166" class="blob-code js-file-line">  <span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L1167" class="blob-num js-line-number" data-line-number="1167"></td>
        <td id="LC1167" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L1168" class="blob-num js-line-number" data-line-number="1168"></td>
        <td id="LC1168" class="blob-code js-file-line">  <span class="k">return</span> <span class="nx">comparison</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L1169" class="blob-num js-line-number" data-line-number="1169"></td>
        <td id="LC1169" class="blob-code js-file-line"><span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L1170" class="blob-num js-line-number" data-line-number="1170"></td>
        <td id="LC1170" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L1171" class="blob-num js-line-number" data-line-number="1171"></td>
        <td id="LC1171" class="blob-code js-file-line"><span class="kd">var</span> <span class="nx">cubism_comparisonPrimaryFormat</span> <span class="o">=</span> <span class="nx">d3</span><span class="p">.</span><span class="nx">format</span><span class="p">(</span><span class="s2">&quot;.2s&quot;</span><span class="p">),</span></td>
      </tr>
      <tr>
        <td id="L1172" class="blob-num js-line-number" data-line-number="1172"></td>
        <td id="LC1172" class="blob-code js-file-line">    <span class="nx">cubism_comparisonChangeFormat</span> <span class="o">=</span> <span class="nx">d3</span><span class="p">.</span><span class="nx">format</span><span class="p">(</span><span class="s2">&quot;+.0%&quot;</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L1173" class="blob-num js-line-number" data-line-number="1173"></td>
        <td id="LC1173" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L1174" class="blob-num js-line-number" data-line-number="1174"></td>
        <td id="LC1174" class="blob-code js-file-line"><span class="kd">function</span> <span class="nx">cubism_comparisonRoundEven</span><span class="p">(</span><span class="nx">i</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L1175" class="blob-num js-line-number" data-line-number="1175"></td>
        <td id="LC1175" class="blob-code js-file-line">  <span class="k">return</span> <span class="nx">i</span> <span class="o">&amp;</span> <span class="mh">0xfffffe</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L1176" class="blob-num js-line-number" data-line-number="1176"></td>
        <td id="LC1176" class="blob-code js-file-line"><span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L1177" class="blob-num js-line-number" data-line-number="1177"></td>
        <td id="LC1177" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L1178" class="blob-num js-line-number" data-line-number="1178"></td>
        <td id="LC1178" class="blob-code js-file-line"><span class="kd">function</span> <span class="nx">cubism_comparisonRoundOdd</span><span class="p">(</span><span class="nx">i</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L1179" class="blob-num js-line-number" data-line-number="1179"></td>
        <td id="LC1179" class="blob-code js-file-line">  <span class="k">return</span> <span class="p">((</span><span class="nx">i</span> <span class="o">+</span> <span class="mi">1</span><span class="p">)</span> <span class="o">&amp;</span> <span class="mh">0xfffffe</span><span class="p">)</span> <span class="o">-</span> <span class="mi">1</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L1180" class="blob-num js-line-number" data-line-number="1180"></td>
        <td id="LC1180" class="blob-code js-file-line"><span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L1181" class="blob-num js-line-number" data-line-number="1181"></td>
        <td id="LC1181" class="blob-code js-file-line"><span class="nx">cubism_contextPrototype</span><span class="p">.</span><span class="nx">axis</span> <span class="o">=</span> <span class="kd">function</span><span class="p">()</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L1182" class="blob-num js-line-number" data-line-number="1182"></td>
        <td id="LC1182" class="blob-code js-file-line">  <span class="kd">var</span> <span class="nx">context</span> <span class="o">=</span> <span class="k">this</span><span class="p">,</span></td>
      </tr>
      <tr>
        <td id="L1183" class="blob-num js-line-number" data-line-number="1183"></td>
        <td id="LC1183" class="blob-code js-file-line">      <span class="nx">scale</span> <span class="o">=</span> <span class="nx">context</span><span class="p">.</span><span class="nx">scale</span><span class="p">,</span></td>
      </tr>
      <tr>
        <td id="L1184" class="blob-num js-line-number" data-line-number="1184"></td>
        <td id="LC1184" class="blob-code js-file-line">      <span class="nx">axis_</span> <span class="o">=</span> <span class="nx">d3</span><span class="p">.</span><span class="nx">svg</span><span class="p">.</span><span class="nx">axis</span><span class="p">().</span><span class="nx">scale</span><span class="p">(</span><span class="nx">scale</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L1185" class="blob-num js-line-number" data-line-number="1185"></td>
        <td id="LC1185" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L1186" class="blob-num js-line-number" data-line-number="1186"></td>
        <td id="LC1186" class="blob-code js-file-line">  <span class="kd">var</span> <span class="nx">formatDefault</span> <span class="o">=</span> <span class="nx">context</span><span class="p">.</span><span class="nx">step</span><span class="p">()</span> <span class="o">&lt;</span> <span class="mi">6</span><span class="nx">e4</span> <span class="o">?</span> <span class="nx">cubism_axisFormatSeconds</span></td>
      </tr>
      <tr>
        <td id="L1187" class="blob-num js-line-number" data-line-number="1187"></td>
        <td id="LC1187" class="blob-code js-file-line">      <span class="o">:</span> <span class="nx">context</span><span class="p">.</span><span class="nx">step</span><span class="p">()</span> <span class="o">&lt;</span> <span class="mi">864</span><span class="nx">e5</span> <span class="o">?</span> <span class="nx">cubism_axisFormatMinutes</span></td>
      </tr>
      <tr>
        <td id="L1188" class="blob-num js-line-number" data-line-number="1188"></td>
        <td id="LC1188" class="blob-code js-file-line">      <span class="o">:</span> <span class="nx">cubism_axisFormatDays</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L1189" class="blob-num js-line-number" data-line-number="1189"></td>
        <td id="LC1189" class="blob-code js-file-line">  <span class="kd">var</span> <span class="nx">format</span> <span class="o">=</span> <span class="nx">formatDefault</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L1190" class="blob-num js-line-number" data-line-number="1190"></td>
        <td id="LC1190" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L1191" class="blob-num js-line-number" data-line-number="1191"></td>
        <td id="LC1191" class="blob-code js-file-line">  <span class="kd">function</span> <span class="nx">axis</span><span class="p">(</span><span class="nx">selection</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L1192" class="blob-num js-line-number" data-line-number="1192"></td>
        <td id="LC1192" class="blob-code js-file-line">    <span class="kd">var</span> <span class="nx">id</span> <span class="o">=</span> <span class="o">++</span><span class="nx">cubism_id</span><span class="p">,</span></td>
      </tr>
      <tr>
        <td id="L1193" class="blob-num js-line-number" data-line-number="1193"></td>
        <td id="LC1193" class="blob-code js-file-line">        <span class="nx">tick</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L1194" class="blob-num js-line-number" data-line-number="1194"></td>
        <td id="LC1194" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L1195" class="blob-num js-line-number" data-line-number="1195"></td>
        <td id="LC1195" class="blob-code js-file-line">    <span class="kd">var</span> <span class="nx">g</span> <span class="o">=</span> <span class="nx">selection</span><span class="p">.</span><span class="nx">append</span><span class="p">(</span><span class="s2">&quot;svg&quot;</span><span class="p">)</span></td>
      </tr>
      <tr>
        <td id="L1196" class="blob-num js-line-number" data-line-number="1196"></td>
        <td id="LC1196" class="blob-code js-file-line">        <span class="p">.</span><span class="nx">datum</span><span class="p">({</span><span class="nx">id</span><span class="o">:</span> <span class="nx">id</span><span class="p">})</span></td>
      </tr>
      <tr>
        <td id="L1197" class="blob-num js-line-number" data-line-number="1197"></td>
        <td id="LC1197" class="blob-code js-file-line">        <span class="p">.</span><span class="nx">attr</span><span class="p">(</span><span class="s2">&quot;width&quot;</span><span class="p">,</span> <span class="nx">context</span><span class="p">.</span><span class="nx">size</span><span class="p">())</span></td>
      </tr>
      <tr>
        <td id="L1198" class="blob-num js-line-number" data-line-number="1198"></td>
        <td id="LC1198" class="blob-code js-file-line">        <span class="p">.</span><span class="nx">attr</span><span class="p">(</span><span class="s2">&quot;height&quot;</span><span class="p">,</span> <span class="nb">Math</span><span class="p">.</span><span class="nx">max</span><span class="p">(</span><span class="mi">28</span><span class="p">,</span> <span class="o">-</span><span class="nx">axis</span><span class="p">.</span><span class="nx">tickSize</span><span class="p">()))</span></td>
      </tr>
      <tr>
        <td id="L1199" class="blob-num js-line-number" data-line-number="1199"></td>
        <td id="LC1199" class="blob-code js-file-line">      <span class="p">.</span><span class="nx">append</span><span class="p">(</span><span class="s2">&quot;g&quot;</span><span class="p">)</span></td>
      </tr>
      <tr>
        <td id="L1200" class="blob-num js-line-number" data-line-number="1200"></td>
        <td id="LC1200" class="blob-code js-file-line">        <span class="p">.</span><span class="nx">attr</span><span class="p">(</span><span class="s2">&quot;transform&quot;</span><span class="p">,</span> <span class="s2">&quot;translate(0,&quot;</span> <span class="o">+</span> <span class="p">(</span><span class="nx">axis_</span><span class="p">.</span><span class="nx">orient</span><span class="p">()</span> <span class="o">===</span> <span class="s2">&quot;top&quot;</span> <span class="o">?</span> <span class="mi">27</span> <span class="o">:</span> <span class="mi">4</span><span class="p">)</span> <span class="o">+</span> <span class="s2">&quot;)&quot;</span><span class="p">)</span></td>
      </tr>
      <tr>
        <td id="L1201" class="blob-num js-line-number" data-line-number="1201"></td>
        <td id="LC1201" class="blob-code js-file-line">        <span class="p">.</span><span class="nx">call</span><span class="p">(</span><span class="nx">axis_</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L1202" class="blob-num js-line-number" data-line-number="1202"></td>
        <td id="LC1202" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L1203" class="blob-num js-line-number" data-line-number="1203"></td>
        <td id="LC1203" class="blob-code js-file-line">    <span class="nx">context</span><span class="p">.</span><span class="nx">on</span><span class="p">(</span><span class="s2">&quot;change.axis-&quot;</span> <span class="o">+</span> <span class="nx">id</span><span class="p">,</span> <span class="kd">function</span><span class="p">()</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L1204" class="blob-num js-line-number" data-line-number="1204"></td>
        <td id="LC1204" class="blob-code js-file-line">      <span class="nx">g</span><span class="p">.</span><span class="nx">call</span><span class="p">(</span><span class="nx">axis_</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L1205" class="blob-num js-line-number" data-line-number="1205"></td>
        <td id="LC1205" class="blob-code js-file-line">      <span class="k">if</span> <span class="p">(</span><span class="o">!</span><span class="nx">tick</span><span class="p">)</span> <span class="nx">tick</span> <span class="o">=</span> <span class="nx">d3</span><span class="p">.</span><span class="nx">select</span><span class="p">(</span><span class="nx">g</span><span class="p">.</span><span class="nx">node</span><span class="p">().</span><span class="nx">appendChild</span><span class="p">(</span><span class="nx">g</span><span class="p">.</span><span class="nx">selectAll</span><span class="p">(</span><span class="s2">&quot;text&quot;</span><span class="p">).</span><span class="nx">node</span><span class="p">().</span><span class="nx">cloneNode</span><span class="p">(</span><span class="kc">true</span><span class="p">)))</span></td>
      </tr>
      <tr>
        <td id="L1206" class="blob-num js-line-number" data-line-number="1206"></td>
        <td id="LC1206" class="blob-code js-file-line">          <span class="p">.</span><span class="nx">style</span><span class="p">(</span><span class="s2">&quot;display&quot;</span><span class="p">,</span> <span class="s2">&quot;none&quot;</span><span class="p">)</span></td>
      </tr>
      <tr>
        <td id="L1207" class="blob-num js-line-number" data-line-number="1207"></td>
        <td id="LC1207" class="blob-code js-file-line">          <span class="p">.</span><span class="nx">text</span><span class="p">(</span><span class="kc">null</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L1208" class="blob-num js-line-number" data-line-number="1208"></td>
        <td id="LC1208" class="blob-code js-file-line">    <span class="p">});</span></td>
      </tr>
      <tr>
        <td id="L1209" class="blob-num js-line-number" data-line-number="1209"></td>
        <td id="LC1209" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L1210" class="blob-num js-line-number" data-line-number="1210"></td>
        <td id="LC1210" class="blob-code js-file-line">    <span class="nx">context</span><span class="p">.</span><span class="nx">on</span><span class="p">(</span><span class="s2">&quot;focus.axis-&quot;</span> <span class="o">+</span> <span class="nx">id</span><span class="p">,</span> <span class="kd">function</span><span class="p">(</span><span class="nx">i</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L1211" class="blob-num js-line-number" data-line-number="1211"></td>
        <td id="LC1211" class="blob-code js-file-line">      <span class="k">if</span> <span class="p">(</span><span class="nx">tick</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L1212" class="blob-num js-line-number" data-line-number="1212"></td>
        <td id="LC1212" class="blob-code js-file-line">        <span class="k">if</span> <span class="p">(</span><span class="nx">i</span> <span class="o">==</span> <span class="kc">null</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L1213" class="blob-num js-line-number" data-line-number="1213"></td>
        <td id="LC1213" class="blob-code js-file-line">          <span class="nx">tick</span><span class="p">.</span><span class="nx">style</span><span class="p">(</span><span class="s2">&quot;display&quot;</span><span class="p">,</span> <span class="s2">&quot;none&quot;</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L1214" class="blob-num js-line-number" data-line-number="1214"></td>
        <td id="LC1214" class="blob-code js-file-line">          <span class="nx">g</span><span class="p">.</span><span class="nx">selectAll</span><span class="p">(</span><span class="s2">&quot;text&quot;</span><span class="p">).</span><span class="nx">style</span><span class="p">(</span><span class="s2">&quot;fill-opacity&quot;</span><span class="p">,</span> <span class="kc">null</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L1215" class="blob-num js-line-number" data-line-number="1215"></td>
        <td id="LC1215" class="blob-code js-file-line">        <span class="p">}</span> <span class="k">else</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L1216" class="blob-num js-line-number" data-line-number="1216"></td>
        <td id="LC1216" class="blob-code js-file-line">          <span class="nx">tick</span><span class="p">.</span><span class="nx">style</span><span class="p">(</span><span class="s2">&quot;display&quot;</span><span class="p">,</span> <span class="kc">null</span><span class="p">).</span><span class="nx">attr</span><span class="p">(</span><span class="s2">&quot;x&quot;</span><span class="p">,</span> <span class="nx">i</span><span class="p">).</span><span class="nx">text</span><span class="p">(</span><span class="nx">format</span><span class="p">(</span><span class="nx">scale</span><span class="p">.</span><span class="nx">invert</span><span class="p">(</span><span class="nx">i</span><span class="p">)));</span></td>
      </tr>
      <tr>
        <td id="L1217" class="blob-num js-line-number" data-line-number="1217"></td>
        <td id="LC1217" class="blob-code js-file-line">          <span class="kd">var</span> <span class="nx">dx</span> <span class="o">=</span> <span class="nx">tick</span><span class="p">.</span><span class="nx">node</span><span class="p">().</span><span class="nx">getComputedTextLength</span><span class="p">()</span> <span class="o">+</span> <span class="mi">6</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L1218" class="blob-num js-line-number" data-line-number="1218"></td>
        <td id="LC1218" class="blob-code js-file-line">          <span class="nx">g</span><span class="p">.</span><span class="nx">selectAll</span><span class="p">(</span><span class="s2">&quot;text&quot;</span><span class="p">).</span><span class="nx">style</span><span class="p">(</span><span class="s2">&quot;fill-opacity&quot;</span><span class="p">,</span> <span class="kd">function</span><span class="p">(</span><span class="nx">d</span><span class="p">)</span> <span class="p">{</span> <span class="k">return</span> <span class="nb">Math</span><span class="p">.</span><span class="nx">abs</span><span class="p">(</span><span class="nx">scale</span><span class="p">(</span><span class="nx">d</span><span class="p">)</span> <span class="o">-</span> <span class="nx">i</span><span class="p">)</span> <span class="o">&lt;</span> <span class="nx">dx</span> <span class="o">?</span> <span class="mi">0</span> <span class="o">:</span> <span class="mi">1</span><span class="p">;</span> <span class="p">});</span></td>
      </tr>
      <tr>
        <td id="L1219" class="blob-num js-line-number" data-line-number="1219"></td>
        <td id="LC1219" class="blob-code js-file-line">        <span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L1220" class="blob-num js-line-number" data-line-number="1220"></td>
        <td id="LC1220" class="blob-code js-file-line">      <span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L1221" class="blob-num js-line-number" data-line-number="1221"></td>
        <td id="LC1221" class="blob-code js-file-line">    <span class="p">});</span></td>
      </tr>
      <tr>
        <td id="L1222" class="blob-num js-line-number" data-line-number="1222"></td>
        <td id="LC1222" class="blob-code js-file-line">  <span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L1223" class="blob-num js-line-number" data-line-number="1223"></td>
        <td id="LC1223" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L1224" class="blob-num js-line-number" data-line-number="1224"></td>
        <td id="LC1224" class="blob-code js-file-line">  <span class="nx">axis</span><span class="p">.</span><span class="nx">remove</span> <span class="o">=</span> <span class="kd">function</span><span class="p">(</span><span class="nx">selection</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L1225" class="blob-num js-line-number" data-line-number="1225"></td>
        <td id="LC1225" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L1226" class="blob-num js-line-number" data-line-number="1226"></td>
        <td id="LC1226" class="blob-code js-file-line">    <span class="nx">selection</span><span class="p">.</span><span class="nx">selectAll</span><span class="p">(</span><span class="s2">&quot;svg&quot;</span><span class="p">)</span></td>
      </tr>
      <tr>
        <td id="L1227" class="blob-num js-line-number" data-line-number="1227"></td>
        <td id="LC1227" class="blob-code js-file-line">        <span class="p">.</span><span class="nx">each</span><span class="p">(</span><span class="nx">remove</span><span class="p">)</span></td>
      </tr>
      <tr>
        <td id="L1228" class="blob-num js-line-number" data-line-number="1228"></td>
        <td id="LC1228" class="blob-code js-file-line">        <span class="p">.</span><span class="nx">remove</span><span class="p">();</span></td>
      </tr>
      <tr>
        <td id="L1229" class="blob-num js-line-number" data-line-number="1229"></td>
        <td id="LC1229" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L1230" class="blob-num js-line-number" data-line-number="1230"></td>
        <td id="LC1230" class="blob-code js-file-line">    <span class="kd">function</span> <span class="nx">remove</span><span class="p">(</span><span class="nx">d</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L1231" class="blob-num js-line-number" data-line-number="1231"></td>
        <td id="LC1231" class="blob-code js-file-line">      <span class="nx">context</span><span class="p">.</span><span class="nx">on</span><span class="p">(</span><span class="s2">&quot;change.axis-&quot;</span> <span class="o">+</span> <span class="nx">d</span><span class="p">.</span><span class="nx">id</span><span class="p">,</span> <span class="kc">null</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L1232" class="blob-num js-line-number" data-line-number="1232"></td>
        <td id="LC1232" class="blob-code js-file-line">      <span class="nx">context</span><span class="p">.</span><span class="nx">on</span><span class="p">(</span><span class="s2">&quot;focus.axis-&quot;</span> <span class="o">+</span> <span class="nx">d</span><span class="p">.</span><span class="nx">id</span><span class="p">,</span> <span class="kc">null</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L1233" class="blob-num js-line-number" data-line-number="1233"></td>
        <td id="LC1233" class="blob-code js-file-line">    <span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L1234" class="blob-num js-line-number" data-line-number="1234"></td>
        <td id="LC1234" class="blob-code js-file-line">  <span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L1235" class="blob-num js-line-number" data-line-number="1235"></td>
        <td id="LC1235" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L1236" class="blob-num js-line-number" data-line-number="1236"></td>
        <td id="LC1236" class="blob-code js-file-line">  <span class="nx">axis</span><span class="p">.</span><span class="nx">focusFormat</span> <span class="o">=</span> <span class="kd">function</span><span class="p">(</span><span class="nx">_</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L1237" class="blob-num js-line-number" data-line-number="1237"></td>
        <td id="LC1237" class="blob-code js-file-line">    <span class="k">if</span> <span class="p">(</span><span class="o">!</span><span class="nx">arguments</span><span class="p">.</span><span class="nx">length</span><span class="p">)</span> <span class="k">return</span> <span class="nx">format</span> <span class="o">==</span> <span class="nx">formatDefault</span> <span class="o">?</span> <span class="kc">null</span> <span class="o">:</span> <span class="nx">_</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L1238" class="blob-num js-line-number" data-line-number="1238"></td>
        <td id="LC1238" class="blob-code js-file-line">    <span class="nx">format</span> <span class="o">=</span> <span class="nx">_</span> <span class="o">==</span> <span class="kc">null</span> <span class="o">?</span> <span class="nx">formatDefault</span> <span class="o">:</span> <span class="nx">_</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L1239" class="blob-num js-line-number" data-line-number="1239"></td>
        <td id="LC1239" class="blob-code js-file-line">    <span class="k">return</span> <span class="nx">axis</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L1240" class="blob-num js-line-number" data-line-number="1240"></td>
        <td id="LC1240" class="blob-code js-file-line">  <span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L1241" class="blob-num js-line-number" data-line-number="1241"></td>
        <td id="LC1241" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L1242" class="blob-num js-line-number" data-line-number="1242"></td>
        <td id="LC1242" class="blob-code js-file-line">  <span class="k">return</span> <span class="nx">d3</span><span class="p">.</span><span class="nx">rebind</span><span class="p">(</span><span class="nx">axis</span><span class="p">,</span> <span class="nx">axis_</span><span class="p">,</span></td>
      </tr>
      <tr>
        <td id="L1243" class="blob-num js-line-number" data-line-number="1243"></td>
        <td id="LC1243" class="blob-code js-file-line">      <span class="s2">&quot;orient&quot;</span><span class="p">,</span></td>
      </tr>
      <tr>
        <td id="L1244" class="blob-num js-line-number" data-line-number="1244"></td>
        <td id="LC1244" class="blob-code js-file-line">      <span class="s2">&quot;ticks&quot;</span><span class="p">,</span></td>
      </tr>
      <tr>
        <td id="L1245" class="blob-num js-line-number" data-line-number="1245"></td>
        <td id="LC1245" class="blob-code js-file-line">      <span class="s2">&quot;tickSubdivide&quot;</span><span class="p">,</span></td>
      </tr>
      <tr>
        <td id="L1246" class="blob-num js-line-number" data-line-number="1246"></td>
        <td id="LC1246" class="blob-code js-file-line">      <span class="s2">&quot;tickSize&quot;</span><span class="p">,</span></td>
      </tr>
      <tr>
        <td id="L1247" class="blob-num js-line-number" data-line-number="1247"></td>
        <td id="LC1247" class="blob-code js-file-line">      <span class="s2">&quot;tickPadding&quot;</span><span class="p">,</span></td>
      </tr>
      <tr>
        <td id="L1248" class="blob-num js-line-number" data-line-number="1248"></td>
        <td id="LC1248" class="blob-code js-file-line">      <span class="s2">&quot;tickFormat&quot;</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L1249" class="blob-num js-line-number" data-line-number="1249"></td>
        <td id="LC1249" class="blob-code js-file-line"><span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L1250" class="blob-num js-line-number" data-line-number="1250"></td>
        <td id="LC1250" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L1251" class="blob-num js-line-number" data-line-number="1251"></td>
        <td id="LC1251" class="blob-code js-file-line"><span class="kd">var</span> <span class="nx">cubism_axisFormatSeconds</span> <span class="o">=</span> <span class="nx">d3</span><span class="p">.</span><span class="nx">time</span><span class="p">.</span><span class="nx">format</span><span class="p">(</span><span class="s2">&quot;%I:%M:%S %p&quot;</span><span class="p">),</span></td>
      </tr>
      <tr>
        <td id="L1252" class="blob-num js-line-number" data-line-number="1252"></td>
        <td id="LC1252" class="blob-code js-file-line">    <span class="nx">cubism_axisFormatMinutes</span> <span class="o">=</span> <span class="nx">d3</span><span class="p">.</span><span class="nx">time</span><span class="p">.</span><span class="nx">format</span><span class="p">(</span><span class="s2">&quot;%I:%M %p&quot;</span><span class="p">),</span></td>
      </tr>
      <tr>
        <td id="L1253" class="blob-num js-line-number" data-line-number="1253"></td>
        <td id="LC1253" class="blob-code js-file-line">    <span class="nx">cubism_axisFormatDays</span> <span class="o">=</span> <span class="nx">d3</span><span class="p">.</span><span class="nx">time</span><span class="p">.</span><span class="nx">format</span><span class="p">(</span><span class="s2">&quot;%B %d&quot;</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L1254" class="blob-num js-line-number" data-line-number="1254"></td>
        <td id="LC1254" class="blob-code js-file-line"><span class="nx">cubism_contextPrototype</span><span class="p">.</span><span class="nx">rule</span> <span class="o">=</span> <span class="kd">function</span><span class="p">()</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L1255" class="blob-num js-line-number" data-line-number="1255"></td>
        <td id="LC1255" class="blob-code js-file-line">  <span class="kd">var</span> <span class="nx">context</span> <span class="o">=</span> <span class="k">this</span><span class="p">,</span></td>
      </tr>
      <tr>
        <td id="L1256" class="blob-num js-line-number" data-line-number="1256"></td>
        <td id="LC1256" class="blob-code js-file-line">      <span class="nx">metric</span> <span class="o">=</span> <span class="nx">cubism_identity</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L1257" class="blob-num js-line-number" data-line-number="1257"></td>
        <td id="LC1257" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L1258" class="blob-num js-line-number" data-line-number="1258"></td>
        <td id="LC1258" class="blob-code js-file-line">  <span class="kd">function</span> <span class="nx">rule</span><span class="p">(</span><span class="nx">selection</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L1259" class="blob-num js-line-number" data-line-number="1259"></td>
        <td id="LC1259" class="blob-code js-file-line">    <span class="kd">var</span> <span class="nx">id</span> <span class="o">=</span> <span class="o">++</span><span class="nx">cubism_id</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L1260" class="blob-num js-line-number" data-line-number="1260"></td>
        <td id="LC1260" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L1261" class="blob-num js-line-number" data-line-number="1261"></td>
        <td id="LC1261" class="blob-code js-file-line">    <span class="kd">var</span> <span class="nx">line</span> <span class="o">=</span> <span class="nx">selection</span><span class="p">.</span><span class="nx">append</span><span class="p">(</span><span class="s2">&quot;div&quot;</span><span class="p">)</span></td>
      </tr>
      <tr>
        <td id="L1262" class="blob-num js-line-number" data-line-number="1262"></td>
        <td id="LC1262" class="blob-code js-file-line">        <span class="p">.</span><span class="nx">datum</span><span class="p">({</span><span class="nx">id</span><span class="o">:</span> <span class="nx">id</span><span class="p">})</span></td>
      </tr>
      <tr>
        <td id="L1263" class="blob-num js-line-number" data-line-number="1263"></td>
        <td id="LC1263" class="blob-code js-file-line">        <span class="p">.</span><span class="nx">attr</span><span class="p">(</span><span class="s2">&quot;class&quot;</span><span class="p">,</span> <span class="s2">&quot;line&quot;</span><span class="p">)</span></td>
      </tr>
      <tr>
        <td id="L1264" class="blob-num js-line-number" data-line-number="1264"></td>
        <td id="LC1264" class="blob-code js-file-line">        <span class="p">.</span><span class="nx">call</span><span class="p">(</span><span class="nx">cubism_ruleStyle</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L1265" class="blob-num js-line-number" data-line-number="1265"></td>
        <td id="LC1265" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L1266" class="blob-num js-line-number" data-line-number="1266"></td>
        <td id="LC1266" class="blob-code js-file-line">    <span class="nx">selection</span><span class="p">.</span><span class="nx">each</span><span class="p">(</span><span class="kd">function</span><span class="p">(</span><span class="nx">d</span><span class="p">,</span> <span class="nx">i</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L1267" class="blob-num js-line-number" data-line-number="1267"></td>
        <td id="LC1267" class="blob-code js-file-line">      <span class="kd">var</span> <span class="nx">that</span> <span class="o">=</span> <span class="k">this</span><span class="p">,</span></td>
      </tr>
      <tr>
        <td id="L1268" class="blob-num js-line-number" data-line-number="1268"></td>
        <td id="LC1268" class="blob-code js-file-line">          <span class="nx">id</span> <span class="o">=</span> <span class="o">++</span><span class="nx">cubism_id</span><span class="p">,</span></td>
      </tr>
      <tr>
        <td id="L1269" class="blob-num js-line-number" data-line-number="1269"></td>
        <td id="LC1269" class="blob-code js-file-line">          <span class="nx">metric_</span> <span class="o">=</span> <span class="k">typeof</span> <span class="nx">metric</span> <span class="o">===</span> <span class="s2">&quot;function&quot;</span> <span class="o">?</span> <span class="nx">metric</span><span class="p">.</span><span class="nx">call</span><span class="p">(</span><span class="nx">that</span><span class="p">,</span> <span class="nx">d</span><span class="p">,</span> <span class="nx">i</span><span class="p">)</span> <span class="o">:</span> <span class="nx">metric</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L1270" class="blob-num js-line-number" data-line-number="1270"></td>
        <td id="LC1270" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L1271" class="blob-num js-line-number" data-line-number="1271"></td>
        <td id="LC1271" class="blob-code js-file-line">      <span class="k">if</span> <span class="p">(</span><span class="o">!</span><span class="nx">metric_</span><span class="p">)</span> <span class="k">return</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L1272" class="blob-num js-line-number" data-line-number="1272"></td>
        <td id="LC1272" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L1273" class="blob-num js-line-number" data-line-number="1273"></td>
        <td id="LC1273" class="blob-code js-file-line">      <span class="kd">function</span> <span class="nx">change</span><span class="p">(</span><span class="nx">start</span><span class="p">,</span> <span class="nx">stop</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L1274" class="blob-num js-line-number" data-line-number="1274"></td>
        <td id="LC1274" class="blob-code js-file-line">        <span class="kd">var</span> <span class="nx">values</span> <span class="o">=</span> <span class="p">[];</span></td>
      </tr>
      <tr>
        <td id="L1275" class="blob-num js-line-number" data-line-number="1275"></td>
        <td id="LC1275" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L1276" class="blob-num js-line-number" data-line-number="1276"></td>
        <td id="LC1276" class="blob-code js-file-line">        <span class="k">for</span> <span class="p">(</span><span class="kd">var</span> <span class="nx">i</span> <span class="o">=</span> <span class="mi">0</span><span class="p">,</span> <span class="nx">n</span> <span class="o">=</span> <span class="nx">context</span><span class="p">.</span><span class="nx">size</span><span class="p">();</span> <span class="nx">i</span> <span class="o">&lt;</span> <span class="nx">n</span><span class="p">;</span> <span class="o">++</span><span class="nx">i</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L1277" class="blob-num js-line-number" data-line-number="1277"></td>
        <td id="LC1277" class="blob-code js-file-line">          <span class="k">if</span> <span class="p">(</span><span class="nx">metric_</span><span class="p">.</span><span class="nx">valueAt</span><span class="p">(</span><span class="nx">i</span><span class="p">))</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L1278" class="blob-num js-line-number" data-line-number="1278"></td>
        <td id="LC1278" class="blob-code js-file-line">            <span class="nx">values</span><span class="p">.</span><span class="nx">push</span><span class="p">(</span><span class="nx">i</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L1279" class="blob-num js-line-number" data-line-number="1279"></td>
        <td id="LC1279" class="blob-code js-file-line">          <span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L1280" class="blob-num js-line-number" data-line-number="1280"></td>
        <td id="LC1280" class="blob-code js-file-line">        <span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L1281" class="blob-num js-line-number" data-line-number="1281"></td>
        <td id="LC1281" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L1282" class="blob-num js-line-number" data-line-number="1282"></td>
        <td id="LC1282" class="blob-code js-file-line">        <span class="kd">var</span> <span class="nx">lines</span> <span class="o">=</span> <span class="nx">selection</span><span class="p">.</span><span class="nx">selectAll</span><span class="p">(</span><span class="s2">&quot;.metric&quot;</span><span class="p">).</span><span class="nx">data</span><span class="p">(</span><span class="nx">values</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L1283" class="blob-num js-line-number" data-line-number="1283"></td>
        <td id="LC1283" class="blob-code js-file-line">        <span class="nx">lines</span><span class="p">.</span><span class="nx">exit</span><span class="p">().</span><span class="nx">remove</span><span class="p">();</span></td>
      </tr>
      <tr>
        <td id="L1284" class="blob-num js-line-number" data-line-number="1284"></td>
        <td id="LC1284" class="blob-code js-file-line">        <span class="nx">lines</span><span class="p">.</span><span class="nx">enter</span><span class="p">().</span><span class="nx">append</span><span class="p">(</span><span class="s2">&quot;div&quot;</span><span class="p">).</span><span class="nx">attr</span><span class="p">(</span><span class="s2">&quot;class&quot;</span><span class="p">,</span> <span class="s2">&quot;metric line&quot;</span><span class="p">).</span><span class="nx">call</span><span class="p">(</span><span class="nx">cubism_ruleStyle</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L1285" class="blob-num js-line-number" data-line-number="1285"></td>
        <td id="LC1285" class="blob-code js-file-line">        <span class="nx">lines</span><span class="p">.</span><span class="nx">style</span><span class="p">(</span><span class="s2">&quot;left&quot;</span><span class="p">,</span> <span class="nx">cubism_ruleLeft</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L1286" class="blob-num js-line-number" data-line-number="1286"></td>
        <td id="LC1286" class="blob-code js-file-line">      <span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L1287" class="blob-num js-line-number" data-line-number="1287"></td>
        <td id="LC1287" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L1288" class="blob-num js-line-number" data-line-number="1288"></td>
        <td id="LC1288" class="blob-code js-file-line">      <span class="nx">context</span><span class="p">.</span><span class="nx">on</span><span class="p">(</span><span class="s2">&quot;change.rule-&quot;</span> <span class="o">+</span> <span class="nx">id</span><span class="p">,</span> <span class="nx">change</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L1289" class="blob-num js-line-number" data-line-number="1289"></td>
        <td id="LC1289" class="blob-code js-file-line">      <span class="nx">metric_</span><span class="p">.</span><span class="nx">on</span><span class="p">(</span><span class="s2">&quot;change.rule-&quot;</span> <span class="o">+</span> <span class="nx">id</span><span class="p">,</span> <span class="nx">change</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L1290" class="blob-num js-line-number" data-line-number="1290"></td>
        <td id="LC1290" class="blob-code js-file-line">    <span class="p">});</span></td>
      </tr>
      <tr>
        <td id="L1291" class="blob-num js-line-number" data-line-number="1291"></td>
        <td id="LC1291" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L1292" class="blob-num js-line-number" data-line-number="1292"></td>
        <td id="LC1292" class="blob-code js-file-line">    <span class="nx">context</span><span class="p">.</span><span class="nx">on</span><span class="p">(</span><span class="s2">&quot;focus.rule-&quot;</span> <span class="o">+</span> <span class="nx">id</span><span class="p">,</span> <span class="kd">function</span><span class="p">(</span><span class="nx">i</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L1293" class="blob-num js-line-number" data-line-number="1293"></td>
        <td id="LC1293" class="blob-code js-file-line">      <span class="nx">line</span><span class="p">.</span><span class="nx">datum</span><span class="p">(</span><span class="nx">i</span><span class="p">)</span></td>
      </tr>
      <tr>
        <td id="L1294" class="blob-num js-line-number" data-line-number="1294"></td>
        <td id="LC1294" class="blob-code js-file-line">          <span class="p">.</span><span class="nx">style</span><span class="p">(</span><span class="s2">&quot;display&quot;</span><span class="p">,</span> <span class="nx">i</span> <span class="o">==</span> <span class="kc">null</span> <span class="o">?</span> <span class="s2">&quot;none&quot;</span> <span class="o">:</span> <span class="kc">null</span><span class="p">)</span></td>
      </tr>
      <tr>
        <td id="L1295" class="blob-num js-line-number" data-line-number="1295"></td>
        <td id="LC1295" class="blob-code js-file-line">          <span class="p">.</span><span class="nx">style</span><span class="p">(</span><span class="s2">&quot;left&quot;</span><span class="p">,</span> <span class="nx">i</span> <span class="o">==</span> <span class="kc">null</span> <span class="o">?</span> <span class="kc">null</span> <span class="o">:</span> <span class="nx">cubism_ruleLeft</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L1296" class="blob-num js-line-number" data-line-number="1296"></td>
        <td id="LC1296" class="blob-code js-file-line">    <span class="p">});</span></td>
      </tr>
      <tr>
        <td id="L1297" class="blob-num js-line-number" data-line-number="1297"></td>
        <td id="LC1297" class="blob-code js-file-line">  <span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L1298" class="blob-num js-line-number" data-line-number="1298"></td>
        <td id="LC1298" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L1299" class="blob-num js-line-number" data-line-number="1299"></td>
        <td id="LC1299" class="blob-code js-file-line">  <span class="nx">rule</span><span class="p">.</span><span class="nx">remove</span> <span class="o">=</span> <span class="kd">function</span><span class="p">(</span><span class="nx">selection</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L1300" class="blob-num js-line-number" data-line-number="1300"></td>
        <td id="LC1300" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L1301" class="blob-num js-line-number" data-line-number="1301"></td>
        <td id="LC1301" class="blob-code js-file-line">    <span class="nx">selection</span><span class="p">.</span><span class="nx">selectAll</span><span class="p">(</span><span class="s2">&quot;.line&quot;</span><span class="p">)</span></td>
      </tr>
      <tr>
        <td id="L1302" class="blob-num js-line-number" data-line-number="1302"></td>
        <td id="LC1302" class="blob-code js-file-line">        <span class="p">.</span><span class="nx">each</span><span class="p">(</span><span class="nx">remove</span><span class="p">)</span></td>
      </tr>
      <tr>
        <td id="L1303" class="blob-num js-line-number" data-line-number="1303"></td>
        <td id="LC1303" class="blob-code js-file-line">        <span class="p">.</span><span class="nx">remove</span><span class="p">();</span></td>
      </tr>
      <tr>
        <td id="L1304" class="blob-num js-line-number" data-line-number="1304"></td>
        <td id="LC1304" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L1305" class="blob-num js-line-number" data-line-number="1305"></td>
        <td id="LC1305" class="blob-code js-file-line">    <span class="kd">function</span> <span class="nx">remove</span><span class="p">(</span><span class="nx">d</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L1306" class="blob-num js-line-number" data-line-number="1306"></td>
        <td id="LC1306" class="blob-code js-file-line">      <span class="nx">context</span><span class="p">.</span><span class="nx">on</span><span class="p">(</span><span class="s2">&quot;focus.rule-&quot;</span> <span class="o">+</span> <span class="nx">d</span><span class="p">.</span><span class="nx">id</span><span class="p">,</span> <span class="kc">null</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L1307" class="blob-num js-line-number" data-line-number="1307"></td>
        <td id="LC1307" class="blob-code js-file-line">    <span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L1308" class="blob-num js-line-number" data-line-number="1308"></td>
        <td id="LC1308" class="blob-code js-file-line">  <span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L1309" class="blob-num js-line-number" data-line-number="1309"></td>
        <td id="LC1309" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L1310" class="blob-num js-line-number" data-line-number="1310"></td>
        <td id="LC1310" class="blob-code js-file-line">  <span class="nx">rule</span><span class="p">.</span><span class="nx">metric</span> <span class="o">=</span> <span class="kd">function</span><span class="p">(</span><span class="nx">_</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L1311" class="blob-num js-line-number" data-line-number="1311"></td>
        <td id="LC1311" class="blob-code js-file-line">    <span class="k">if</span> <span class="p">(</span><span class="o">!</span><span class="nx">arguments</span><span class="p">.</span><span class="nx">length</span><span class="p">)</span> <span class="k">return</span> <span class="nx">metric</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L1312" class="blob-num js-line-number" data-line-number="1312"></td>
        <td id="LC1312" class="blob-code js-file-line">    <span class="nx">metric</span> <span class="o">=</span> <span class="nx">_</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L1313" class="blob-num js-line-number" data-line-number="1313"></td>
        <td id="LC1313" class="blob-code js-file-line">    <span class="k">return</span> <span class="nx">rule</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L1314" class="blob-num js-line-number" data-line-number="1314"></td>
        <td id="LC1314" class="blob-code js-file-line">  <span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L1315" class="blob-num js-line-number" data-line-number="1315"></td>
        <td id="LC1315" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L1316" class="blob-num js-line-number" data-line-number="1316"></td>
        <td id="LC1316" class="blob-code js-file-line">  <span class="k">return</span> <span class="nx">rule</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L1317" class="blob-num js-line-number" data-line-number="1317"></td>
        <td id="LC1317" class="blob-code js-file-line"><span class="p">};</span></td>
      </tr>
      <tr>
        <td id="L1318" class="blob-num js-line-number" data-line-number="1318"></td>
        <td id="LC1318" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L1319" class="blob-num js-line-number" data-line-number="1319"></td>
        <td id="LC1319" class="blob-code js-file-line"><span class="kd">function</span> <span class="nx">cubism_ruleStyle</span><span class="p">(</span><span class="nx">line</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L1320" class="blob-num js-line-number" data-line-number="1320"></td>
        <td id="LC1320" class="blob-code js-file-line">  <span class="nx">line</span></td>
      </tr>
      <tr>
        <td id="L1321" class="blob-num js-line-number" data-line-number="1321"></td>
        <td id="LC1321" class="blob-code js-file-line">      <span class="p">.</span><span class="nx">style</span><span class="p">(</span><span class="s2">&quot;position&quot;</span><span class="p">,</span> <span class="s2">&quot;absolute&quot;</span><span class="p">)</span></td>
      </tr>
      <tr>
        <td id="L1322" class="blob-num js-line-number" data-line-number="1322"></td>
        <td id="LC1322" class="blob-code js-file-line">      <span class="p">.</span><span class="nx">style</span><span class="p">(</span><span class="s2">&quot;top&quot;</span><span class="p">,</span> <span class="mi">0</span><span class="p">)</span></td>
      </tr>
      <tr>
        <td id="L1323" class="blob-num js-line-number" data-line-number="1323"></td>
        <td id="LC1323" class="blob-code js-file-line">      <span class="p">.</span><span class="nx">style</span><span class="p">(</span><span class="s2">&quot;bottom&quot;</span><span class="p">,</span> <span class="mi">0</span><span class="p">)</span></td>
      </tr>
      <tr>
        <td id="L1324" class="blob-num js-line-number" data-line-number="1324"></td>
        <td id="LC1324" class="blob-code js-file-line">      <span class="p">.</span><span class="nx">style</span><span class="p">(</span><span class="s2">&quot;width&quot;</span><span class="p">,</span> <span class="s2">&quot;1px&quot;</span><span class="p">)</span></td>
      </tr>
      <tr>
        <td id="L1325" class="blob-num js-line-number" data-line-number="1325"></td>
        <td id="LC1325" class="blob-code js-file-line">      <span class="p">.</span><span class="nx">style</span><span class="p">(</span><span class="s2">&quot;pointer-events&quot;</span><span class="p">,</span> <span class="s2">&quot;none&quot;</span><span class="p">);</span></td>
      </tr>
      <tr>
        <td id="L1326" class="blob-num js-line-number" data-line-number="1326"></td>
        <td id="LC1326" class="blob-code js-file-line"><span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L1327" class="blob-num js-line-number" data-line-number="1327"></td>
        <td id="LC1327" class="blob-code js-file-line">
</td>
      </tr>
      <tr>
        <td id="L1328" class="blob-num js-line-number" data-line-number="1328"></td>
        <td id="LC1328" class="blob-code js-file-line"><span class="kd">function</span> <span class="nx">cubism_ruleLeft</span><span class="p">(</span><span class="nx">i</span><span class="p">)</span> <span class="p">{</span></td>
      </tr>
      <tr>
        <td id="L1329" class="blob-num js-line-number" data-line-number="1329"></td>
        <td id="LC1329" class="blob-code js-file-line">  <span class="k">return</span> <span class="nx">i</span> <span class="o">+</span> <span class="s2">&quot;px&quot;</span><span class="p">;</span></td>
      </tr>
      <tr>
        <td id="L1330" class="blob-num js-line-number" data-line-number="1330"></td>
        <td id="LC1330" class="blob-code js-file-line"><span class="p">}</span></td>
      </tr>
      <tr>
        <td id="L1331" class="blob-num js-line-number" data-line-number="1331"></td>
        <td id="LC1331" class="blob-code js-file-line"><span class="p">})(</span><span class="k">this</span><span class="p">);</span></td>
      </tr>
</table>

  </div>

  </div>
</div>

<a href="#jump-to-line" rel="facebox[.linejump]" data-hotkey="l" style="display:none">Jump to Line</a>
<div id="jump-to-line" style="display:none">
  <form accept-charset="UTF-8" class="js-jump-to-line-form">
    <input class="linejump-input js-jump-to-line-field" type="text" placeholder="Jump to line&hellip;" autofocus>
    <button type="submit" class="button">Go</button>
  </form>
</div>

        </div>

      </div><!-- /.repo-container -->
      <div class="modal-backdrop"></div>
    </div><!-- /.container -->
  </div><!-- /.site -->


    </div><!-- /.wrapper -->

      <div class="container">
  <div class="site-footer" role="contentinfo">
    <ul class="site-footer-links right">
      <li><a href="https://status.github.com/">Status</a></li>
      <li><a href="http://developer.github.com">API</a></li>
      <li><a href="http://training.github.com">Training</a></li>
      <li><a href="http://shop.github.com">Shop</a></li>
      <li><a href="/blog">Blog</a></li>
      <li><a href="/about">About</a></li>

    </ul>

    <a href="/" aria-label="Homepage">
      <span class="mega-octicon octicon-mark-github" title="GitHub"></span>
    </a>

    <ul class="site-footer-links">
      <li>&copy; 2014 <span title="0.06547s from github-fe126-cp1-prd.iad.github.net">GitHub</span>, Inc.</li>
        <li><a href="/site/terms">Terms</a></li>
        <li><a href="/site/privacy">Privacy</a></li>
        <li><a href="/security">Security</a></li>
        <li><a href="/contact">Contact</a></li>
    </ul>
  </div><!-- /.site-footer -->
</div><!-- /.container -->


    <div class="fullscreen-overlay js-fullscreen-overlay" id="fullscreen_overlay">
  <div class="fullscreen-container js-suggester-container">
    <div class="textarea-wrap">
      <textarea name="fullscreen-contents" id="fullscreen-contents" class="fullscreen-contents js-fullscreen-contents js-suggester-field" placeholder=""></textarea>
    </div>
  </div>
  <div class="fullscreen-sidebar">
    <a href="#" class="exit-fullscreen js-exit-fullscreen tooltipped tooltipped-w" aria-label="Exit Zen Mode">
      <span class="mega-octicon octicon-screen-normal"></span>
    </a>
    <a href="#" class="theme-switcher js-theme-switcher tooltipped tooltipped-w"
      aria-label="Switch themes">
      <span class="octicon octicon-color-mode"></span>
    </a>
  </div>
</div>



    <div id="ajax-error-message" class="flash flash-error">
      <span class="octicon octicon-alert"></span>
      <a href="#" class="octicon octicon-x flash-close js-ajax-error-dismiss" aria-label="Dismiss error"></a>
      Something went wrong with that request. Please try again.
    </div>


      <script crossorigin="anonymous" src="https://assets-cdn.github.com/assets/frameworks-baafc7fe2c3ef575c8bcd5ae98c322c1e333a6c520e64e31b8a56a462e252006.js" type="text/javascript"></script>
      <script async="async" crossorigin="anonymous" src="https://assets-cdn.github.com/assets/github-b5b13912747fbf959193da31b1678b2a0cd890ab41baba08a8b1a90d5dcbc57f.js" type="text/javascript"></script>
      
      
        <script async src="https://www.google-analytics.com/analytics.js"></script>
  </body>
</html>

