# system

Access system-level information (under construction).

```js
system
```

## Methods
```js
system.getTheme()
```

Returns: `Promise<string>` - Resolves as a string containing the current system theme, which is one of `dark`, `contrast`, `inverted`, `default`

Request the current system theme.
<br><br>
```js
system.toggleDevTools()
```

Toggles developer tools.