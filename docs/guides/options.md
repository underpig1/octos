# Adding user options to your mod

Adding user options to your mod is a great way to allow users to configure or customize your mod in the Octos app:

<img src="../../img/user-options.png" width="150px" aria-hidden />

In this guide, we will go through adding some user options to an example mod.

We need to add an [`options` field](config.md#options) to our [`octos.json` file](config.md), which is where we'll specify the actual inputs to add, and then use the [`UserOptions`](../../reference/useroptions/) class of the Octos API to listen to changes in those settings by the user.

### Adding user options
Recall that your mod can optionally include an [`octos.json`](config.md) file in the root directory to specify metadata:
```
MyAwesomeMod/
├── index.html
├── octos.json
```
In our `octos.json`, we can include the [`options` field](config.md#options) to specify which options we want to add. For example, let's add a range slider for controlling the speed of something within our mod:
```json
{
    "name": "MyAwesomeMod",
    "options": {
        "speed": { // a range slider with ID 'speed'
            "label": "My speed:", // the display name (required to render)
            "type": "range",
            "value": 5, // our default value
            "step": 5, // the allowed interval between values
            "min": 0, // the minimum allowed value
            "max": 5 // the maximum allowed value
        }
        // you can add other options here
    }
}
```
You can see an example of a slider UI in the image at the top of this page. To see all the different possible types of options you can add, see the [octos.json reference](config.md#options).

A couple things to note in this example:

- The keys of the `options` field correspond to IDs that we will use to refer to our options later in JavaScript

- A `label` is required for each option in order for it to render

- You can add as many options as you want from a wide range of potential inputs, including color pickers, number inputs, file selectors, checkboxes, dropdown menus, and more

Great! Now if you zipped your mod folder and installed it through the Octos app, you would see a new speed range appear in the sidebar when you click on your mod. Now, it's time to actually listen to changes in our speed range.

### Listening to changes in user options
First, make sure to add the Octos API to your mod using one of the following methods:

**Option 1: CDN (recommended)**

Include this script tag in your HTML:
```html
<script src="https://cdn.jsdelivr.net/npm/octos@latest/octos.min.js"></script>
```

**Option 2: npm**

You can also install the Octos API with npm:
```
npm -i octos
```

**Option 3: direct download**
Alternatively, you can download [`octos.min.js`](https://cdn.jsdelivr.net/npm/octos@latest/octos.min.js) from the source and include it directly in your wallpaper's source folder:
```html
<script src="octos.min.js"></script>
``` 

Alright, now we can access the `UserOptions` class in our JavaScript:
```js
const userOptions = new octos.UserOptions();
```

Now, we can listen to our two user options events:

1) `load` event: fired when your mod instance loads

2) `change` event: fired when the user changes one of your options

For both of these events, we can use `userOptions.on(event, callback)`. The `callback` we pass to will be an event handler that recieves an `{id, value}` object. This object includes the ID of the input that has been changed (or loaded), as we specified in our `octos.json` and the new value of the option.

Let's add a handler for both of these events:
```js
const userOptions = new octos.UserOptions();

let speedVariable = 0;

// make an event listener
const handleOptions = ({ id, value }) => { // an (id, value) pair is passed to both events
    switch (id) {
        case 'speed': // this is the id
            speedVariable = parseInt(value) // it's good practice to convert it to an integer just in case
            updateSpeed(); // do what we need with this new variable
            break;
        // add cases for other input ids
        default:
            break;
    }
};

// register our event listener
userOptions.on('change', handleOptions)
userOptions.on('load', handleOptions)
```

Handling both the `change` and `load` events is often useful for separating how we handle initial user option values and then dealing with changes later, but in this example we didn't have a reason to separate them. In this case then, `UserOptions` offers a handy `changeload` event that lets us handle both events at once. We can use this single event handler in place of the last two lines of our previous example:
```js
userOptions.on('changeload', handleOptions)
```

Well done! You've added a user option to your mod.

### Another way
We could also handle user options events without needing to include the `UserOptions` class or even the Octos API in our project. We do this by directly specifying `onchange` and `onload` events in our `options` object. These allow us to give Octos a function to be called on these events. By default, the `value` of the changed or loaded input is passed into the function we specify.

In this example, let's add a text option that will allow the user to select the background color.

Once again, we could separately specify handlers for `onchange` and `onload` events, but let's just combine them into the handy `onchangeload` event.

Let's add our new background color picker to our `octos.json` file:
```json
"options": {
    "url": {
        "label": "Background color:",
        "type": "color-picker",
        "value": "#ffffff",
        "onchangeload": "updateBackground"
    }
    ...
}
```

Then in our `index.js`:
```js
function updateBackground(value) {
    const backgroundColor = value;
    document.body.style.backgroundColor = backgroundColor; // set background color
}
```

We can also use arrow functions to be a bit more concise:
```json
"onchangeload": "(value) => document.body.style.backgroundColor = value"
```

This is similar to how you would add an `onchange` event for an HTML element:
```html
<input type="color" onchange="updateBackground()" />
```

In this example we didn't even need to include the Octos API to add functionality to our options, but there are some advantages to using the `UserOptions` class.

### Next steps
Check out the [`octos.json` documentation for the `options` field](config.md#options) to see the different types of input you can add.

Also see the [`UserOptions`](../../reference/useroptions/) reference to see more methods relating to user options, like dynamically getting and setting user options at any time.