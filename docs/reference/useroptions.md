<a name="UserOptions"></a>

## UserOptions
**Kind**: global class  

* [UserOptions](#UserOptions)
    * [new UserOptions()](#new_UserOptions_new)
    * [.on(eventName, callback)](#UserOptions+on)
    * [.once(eventName, callback)](#UserOptions+once)
    * [.off(eventName, callback)](#UserOptions+off)
    * [.requestOptions()](#UserOptions+requestOptions) ⇒ <code>Promise.&lt;object&gt;</code>
    * [.setOption(id, value)](#UserOptions+setOption)

<a name="new_UserOptions_new"></a>

### new UserOptions()
Handle events relating to changes in user options for your mod.User options are defined in the [`options` object in your octos.json file](config.md#options), and are configurable by users in the Octos app:<img width=200 src="../../img/user-options.png" aria-hidden />

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

<a name="UserOptions+requestOptions"></a>

### userOptions.requestOptions() ⇒ <code>Promise.&lt;object&gt;</code>
Request all user options along with their properties and values and await a response.

**Kind**: instance method of [<code>UserOptions</code>](#UserOptions)  
**Returns**: <code>Promise.&lt;object&gt;</code> - Resolves to an object containing `options` from `octos.json` along with user-set values.  
**Example**  
```jsuserOptions.requestOptions().then((options) => {// what the options object looks like:// {//   "dark-mode": {//     "type": "checkbox",//     "value": (user-set),//     "label": "Enable dark mode?"//   }//   ... any other options// }    if (options['dark-mode'].value) {         myElement.classList.add('dark-mode')    }});```
<a name="UserOptions+setOption"></a>

### userOptions.setOption(id, value)
Set the value of a user option. Note that this will not trigger user events such as `'change'`.

**Kind**: instance method of [<code>UserOptions</code>](#UserOptions)  

| Param | Type | Description |
| --- | --- | --- |
| id | <code>string</code> | The id of the option to set, as specified in `options` of `octos.json`. |
| value | <code>any</code> | The value to set. |

**Example**  
```js
userOptions.setOption('dark-mode', true);
```
