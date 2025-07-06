<a name="UserOptions"></a>

## UserOptions
Handle events relating to changes in user options for your mod.User options are defined in the [`options` object in your octos.json file](config.md#options), and are configurable by users in the Octos app:<img height="300px" src="docs/_media/user-options.png" aria-hidden />

**Kind**: global class  

* [UserOptions](#UserOptions)
    * [.on(eventName, callback)](#UserOptions+on)
    * [.once(eventName, callback)](#UserOptions+once)
    * [.off(eventName, callback)](#UserOptions+off)
    * [.request()](#UserOptions+request) ⇒ <code>Promise.&lt;any&gt;</code>

<a name="UserOptions+on"></a>

### userOptions.on(eventName, callback)
Add an event listener.

**Kind**: instance method of [<code>UserOptions</code>](#UserOptions)  

| Param | Type | Description |
| --- | --- | --- |
| eventName | <code>&#x27;change&#x27;</code> \| <code>&#x27;load&#x27;</code> | Events with type `change` are fired whenever a user changes an option. `load` events are fired when the wallpaper first loads in. |
| callback | <code>function</code> | Callback receives an object containing the ID of the affected option (as specified in `octos.json`) along with its value in the form `{id, value}`. |

**Example**  
```js
// This example shows how to add a simple checkbox for dark mode in your mod and listen for changes by the user.// In your octos.json:{... "options": {   "dark-mode": {     "type": "checkbox",     "value": true,     "label": "Enable dark mode?"   } }...}// In your script.js:// Listen for changes in user options for your moduserOptions.on('change', ({id, value}) => {  if (id == 'dark-mode') {     if (value)         myElement.classList.add('dark-mode')      }      else {          myElement.classList.remove('dark-mode')      }  }  ... // handle changes for other options})
```
<a name="UserOptions+once"></a>

### userOptions.once(eventName, callback)
Add a one-time event listener that removes itself after firing.

**Kind**: instance method of [<code>UserOptions</code>](#UserOptions)  

| Param | Type |
| --- | --- |
| eventName | <code>&#x27;change&#x27;</code> \| <code>&#x27;load&#x27;</code> | 
| callback | <code>function</code> | 

<a name="UserOptions+off"></a>

### userOptions.off(eventName, callback)
Remove an event listener.

**Kind**: instance method of [<code>UserOptions</code>](#UserOptions)  

| Param | Type |
| --- | --- |
| eventName | <code>&#x27;change&#x27;</code> \| <code>&#x27;load&#x27;</code> | 
| callback | <code>function</code> | 

<a name="UserOptions+request"></a>

### userOptions.request() ⇒ <code>Promise.&lt;any&gt;</code>
Request all user options and await a response.

**Kind**: instance method of [<code>UserOptions</code>](#UserOptions)  
**Returns**: <code>Promise.&lt;any&gt;</code> - Resolves with an object containing the entire `options` object.  
**Example**  
```jsuserOptions.request().then((options) => {    if (options['dark-mode'].value) {         myElement.classList.add('dark-mode')    }});```
