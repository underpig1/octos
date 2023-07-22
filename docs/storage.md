# Storage

Read and write to local storage to preserve data across sessions.

```js
new Storage()
```

## Methods
```js
store.write(id, content)
```

- `id` string - The ID of the storage to write
- `content` Object - The data to write to the storage at the given ID

Writes data to the given ID. Any data written throughout a session will persist even when the mod is inactive. The data will be lost, however, if the user removes the mod from their library.
<br><br>
```js
store.read(id)
```

- `id` string - The ID of the storage to read

Returns: `Promise<Object>` - Resolves with the data stored at the given ID.

Reads the data stored with `store.write` at the given ID.

## Examples

Reads and updates the user's "score":

```js
var userScore = 0;
const store = new octos.Storage();

store.read("my-storage").then((data) => {
    if (data) userScore = data.score;
});

function updateScore() {
    userScore++;
    store.write("my-storage", { score: userScore });
}
```