# system

Access system-level information (under construction).

```js
system
```

## Methods
```js
system.getTheme()
```

Returns: `Promise<string>` - Resolves as a string containing the current system theme, which is one of `"dark"`, `"contrast"`, `"inverted"`, `"default"`

Requests the current system theme.
<br><br>
```js
system.toggleDevTools()
```

Toggles developer tools.
<br><br>
```js
system.getCpus()
```

Returns: `Promise<Object[]>` - Resolves as an array of objects containing the information for each CPU in the following format:
- `model` string - CPU model
- `speed` number - Speed of the CPU in ms
- `times` Object - CPU times
   - `user` number - Time spent in user mode in ms
   - `nice` number - Time spent in nice mode in ms
   - `sys` number - Time spent in sys mode in ms
   - `idle` number - Time spent in idle mode in ms
   - `irq` number - Time spent in irq mode in ms


Requests the CPU information for the current device.
<br><br>
```js
system.getArch()
```

Returns: `Promise<string>` - Resolves as a string containing the device's system architecture, one of `"arm"`, `"arm64"`, `"ia32"`, `"mips"`, `"mipsel"`, `"ppc"`, `"ppc64"`, `"s390"`, `"s390x"`, and `"x64"`

Requests the system architecture.
<br><br>
```js
system.getMachine()
```

Returns: `Promise<string>` - Resolves as a string containing the device's machine name, one of `"arm"`, `"arm64"`, `"aarch64"`, `"mips"`, `"mips64"`, `"ppc64"`, `"ppc64le"`, `"s390"`, `"s390x"`, `"i386"`, `"i686"`, `"x86_64"`

Requests the device's machine name.
<br><br>
```js
system.getOS()
```

Returns: `Promise<string>` - Resolves as a string containing the device's operating system name

Requests the device's operating system name.
<br><br>
```js
system.getTotalMem()
```

Returns: `Promise<number>` - Resolves as the device's total memory in bytes

Requests the device's total system memory.
<br><br>
```js
system.getFreeMem()
```

Returns: `Promise<number>` - Resolves as the device's free memory in bytes

Requests the device's free system memory.
<br><br>
```js
system.getUptime()
```

Returns: `Promise<number>` - Resolves as the device's uptime in seconds

Requests the device's uptime.