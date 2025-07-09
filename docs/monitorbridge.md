## Classes

<dl>
<dt><a href="#MonitorBridge">MonitorBridge</a></dt>
<dd><p>Send and recieve messages between instances of your wallpaper across monitors if your wallpaper is running in a multimonitor environment.
Each instance of your wallpaper is assigned to a unique monitor and thus has a unique monitor ID string (something like <code>&quot;\\\\.\\DISPLAY1&quot;</code>). You can use these IDs to send messages between separate instances of your wallpaper.
This is useful for coordinating events between different instances if you want your mod to behave differently across monitors.</p>
</dd>
</dl>

## Typedefs

<dl>
<dt><a href="#MonitorMessage">MonitorMessage</a> : <code>Object</code></dt>
<dd></dd>
</dl>

<a name="MonitorBridge"></a>

## MonitorBridge
Send and recieve messages between instances of your wallpaper across monitors if your wallpaper is running in a multimonitor environment.Each instance of your wallpaper is assigned to a unique monitor and thus has a unique monitor ID string (something like `"\\\\.\\DISPLAY1"`). You can use these IDs to send messages between separate instances of your wallpaper.This is useful for coordinating events between different instances if you want your mod to behave differently across monitors.

**Kind**: global class  

* [MonitorBridge](#MonitorBridge)
    * [.requestId()](#MonitorBridge+requestId) ⇒ <code>Promise.&lt;string&gt;</code>
    * [.requestSiblingIds()](#MonitorBridge+requestSiblingIds) ⇒ <code>Promise.&lt;Array.&lt;string&gt;&gt;</code>
    * [.send(recipientId, data)](#MonitorBridge+send)
    * [.on(eventName, callback)](#MonitorBridge+on)
    * [.once(eventName, callback)](#MonitorBridge+once)
    * [.off(eventName, callback)](#MonitorBridge+off)

<a name="MonitorBridge+requestId"></a>

### monitorBridge.requestId() ⇒ <code>Promise.&lt;string&gt;</code>
Request this window's monitor ID.

**Kind**: instance method of [<code>MonitorBridge</code>](#MonitorBridge)  
**Returns**: <code>Promise.&lt;string&gt;</code> - Resolves to a string containing this window's monitor ID.  
**Example**  
```js
monitorBridge.requestId().then((myId) => {     console.log('My monitor id is: ' + myId); // probably will look like "\\\\.\\DISPLAY1"});
```
<a name="MonitorBridge+requestSiblingIds"></a>

### monitorBridge.requestSiblingIds() ⇒ <code>Promise.&lt;Array.&lt;string&gt;&gt;</code>
Request the monitor IDs of sibling windows. A sibling is another window running on another monitor but running the same mod instance as yours. That is, it will include wallpapers in other monitors only if they are running your wallpaper. Monitors assigned to wallpapers are excluded.

**Kind**: instance method of [<code>MonitorBridge</code>](#MonitorBridge)  
**Returns**: <code>Promise.&lt;Array.&lt;string&gt;&gt;</code> - Resolves to an array of strings containing the monitor ID strings of this window's siblings.  
**Example**  
```js
monitorBridge.requestSiblingIds().then((siblingIds) => {     for (const siblingId of siblingIds) {         console.log('Sibling ID: ' + siblingId);         // do something with each one     }     console.log('We have ' + siblingIds.length + ' simultaneous instances of our wallpaper running on different monitors');});
```
<a name="MonitorBridge+send"></a>

### monitorBridge.send(recipientId, data)
Send a message to a window belonging to another monitor.

**Kind**: instance method of [<code>MonitorBridge</code>](#MonitorBridge)  

| Param | Type | Description |
| --- | --- | --- |
| recipientId | <code>string</code> | The monitor ID of the intended recipient. |
| data | <code>Object</code> | The contents of the message to send. |

**Example**  
```js
// Send a message to all siblingssomeData = {     hello: true,     myFavoriteNumber: 4};monitorBridge.requestSiblingIds().then((siblingIds) => {     siblingIds.forEach((id) => {         monitorBridge.send(id, someData);     });})
```
<a name="MonitorBridge+on"></a>

### monitorBridge.on(eventName, callback)
Add an event listener.

**Kind**: instance method of [<code>MonitorBridge</code>](#MonitorBridge)  

| Param | Type | Description |
| --- | --- | --- |
| eventName | <code>&#x27;message&#x27;</code> | `'message'` events fire when another instance sends a message to this monitor ID. |
| callback | <code>function</code> | Resolves to a [MonitorMessage](#monitormessage) object, containing both `senderId` and `message`. |

**Example**  
```js
monitorBridge.on('message', (e) => {     console.log('Incoming message from: ' + e.senderId);     console.log('Message contents: ' + JSON.stringify(e.message));     // You can also respond:     const recipientId = e.senderId;     monitorBridge.send(recipientId, {         someString: 'I got your message',         someData: 3     });})
```
<a name="MonitorBridge+once"></a>

### monitorBridge.once(eventName, callback)
Add a one-time event listener that removes itself after firing.

**Kind**: instance method of [<code>MonitorBridge</code>](#MonitorBridge)  

| Param | Type |
| --- | --- |
| eventName | <code>&#x27;message&#x27;</code> | 
| callback | <code>function</code> | 

<a name="MonitorBridge+off"></a>

### monitorBridge.off(eventName, callback)
Remove an event listener.

**Kind**: instance method of [<code>MonitorBridge</code>](#MonitorBridge)  

| Param | Type |
| --- | --- |
| eventName | <code>&#x27;message&#x27;</code> | 
| callback | <code>function</code> | 

<a name="MonitorMessage"></a>

## MonitorMessage : <code>Object</code>
**Kind**: global typedef  
**Properties**

| Name | Type | Description |
| --- | --- | --- |
| senderId | <code>string</code> | The monitor ID of the sender. |
| data | <code>Object</code> | The contents of the message being recieved. |

