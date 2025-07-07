<a name="WebView"></a>

## WebView
Configure the WebView2 session for debugging and other options.

**Kind**: global class  

* [WebView](#WebView)
    * [.enableDevTools()](#WebView+enableDevTools)
    * [.getSource()](#WebView+getSource) ⇒ <code>Promise.&lt;string&gt;</code>

<a name="WebView+enableDevTools"></a>

### webView.enableDevTools()
Open the DevTools window to debug your mod.

**Kind**: instance method of [<code>WebView</code>](#WebView)  
**Example**  
```js
const webview = new octos.WebView();webview.enableDevTools();
```
<a name="WebView+getSource"></a>

### webView.getSource() ⇒ <code>Promise.&lt;string&gt;</code>
Get the source uri of the current document.

**Kind**: instance method of [<code>WebView</code>](#WebView)  
**Returns**: <code>Promise.&lt;string&gt;</code> - Resolves to the document's uri.  
