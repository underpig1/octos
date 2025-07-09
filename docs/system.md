<a name="System"></a>

## System
Configure system options and request system information.

**Kind**: global class  

* [System](#System)
    * [.enableDevTools()](#System+enableDevTools)
    * [.getSource()](#System+getSource) ⇒ <code>Promise.&lt;string&gt;</code>
    * [.getTheme()](#System+getTheme) ⇒ <code>Promise.&lt;string&gt;</code>
    * [.setVisibility(state)](#System+setVisibility)
    * [.getVisibility()](#System+getVisibility) ⇒ <code>Promise.&lt;bool&gt;</code>
    * [.setDesktopIconVisibility(state)](#System+setDesktopIconVisibility)
    * [.getDesktopIconVisibility()](#System+getDesktopIconVisibility) ⇒ <code>Promise.&lt;bool&gt;</code>

<a name="System+enableDevTools"></a>

### system.enableDevTools()
Open the DevTools window for mod debugging.

**Kind**: instance method of [<code>System</code>](#System)  
**Example**  
```js
const system = new octos.System();system.enableDevTools();
```
<a name="System+getSource"></a>

### system.getSource() ⇒ <code>Promise.&lt;string&gt;</code>
Get the source uri of the current document.

**Kind**: instance method of [<code>System</code>](#System)  
**Returns**: <code>Promise.&lt;string&gt;</code> - Resolves to the document's uri.  
**Example**  
```js
system.getSource().then((uri) => alert(uri))
```
<a name="System+getTheme"></a>

### system.getTheme() ⇒ <code>Promise.&lt;string&gt;</code>
Get the system theme (light or dark).

**Kind**: instance method of [<code>System</code>](#System)  
**Returns**: <code>Promise.&lt;string&gt;</code> - Resolves to either `'light'` or `'dark'`.  
**Example**  
```js
system.getTheme().then((theme) => alert(theme))
```
<a name="System+setVisibility"></a>

### system.setVisibility(state)
Set the wallpaper's visibility.

**Kind**: instance method of [<code>System</code>](#System)  

| Param | Type | Description |
| --- | --- | --- |
| state | <code>bool</code> | `true` for visible, `false` for hidden. |

<a name="System+getVisibility"></a>

### system.getVisibility() ⇒ <code>Promise.&lt;bool&gt;</code>
Request the wallpaper's visibility.

**Kind**: instance method of [<code>System</code>](#System)  
**Returns**: <code>Promise.&lt;bool&gt;</code> - Resolves to `true` for visible, `false` for hidden.  
<a name="System+setDesktopIconVisibility"></a>

### system.setDesktopIconVisibility(state)
Set the visibility of desktop icons. Useful for hiding the desktop icons when mouse input is expected and then re-enabling them when finished.

**Kind**: instance method of [<code>System</code>](#System)  

| Param | Type | Description |
| --- | --- | --- |
| state | <code>bool</code> | `true` for visible, `false` for hidden. |

<a name="System+getDesktopIconVisibility"></a>

### system.getDesktopIconVisibility() ⇒ <code>Promise.&lt;bool&gt;</code>
Request the visibility of desktop icons.

**Kind**: instance method of [<code>System</code>](#System)  
**Returns**: <code>Promise.&lt;bool&gt;</code> - Resolves to `true` for visible, `false` for hidden.  
