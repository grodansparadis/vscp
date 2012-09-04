/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package org.me.hellodroid;

import android.app.Activity;

import android.os.Bundle;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.view.KeyEvent;
/**
 *
 * @author ahedman1
 */
public class MainActivity extends Activity {

    private class HelloWebViewClient extends WebViewClient {
        @Override
        public boolean shouldOverrideUrlLoading(WebView view, String url) {
            view.loadUrl(url);
            return true;
        }
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if ((keyCode == KeyEvent.KEYCODE_BACK) && mWebView.canGoBack()) {
            mWebView.goBack();
            return true;
        }
        return super.onKeyDown(keyCode, event);
    }

    WebView mWebView;
    
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        setContentView(R.layout.main);

        mWebView = (WebView) findViewById(R.id.webview);
        mWebView.getSettings().setJavaScriptEnabled(true);
        mWebView.setWebViewClient(new HelloWebViewClient());
        mWebView.loadUrl("http://dev.sencha.com/deploy/touch/examples/");

    }

}




