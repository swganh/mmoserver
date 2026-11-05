MMOServer emulates the server side only. You need your own Star Wars
Galaxies client from a legitimate installation, patched to the pre-CU era
(publish 14.1, as the original project required). The project does not
distribute the client or its files, and does not support pirated copies.

## Pointing the client at your server

The client reads the login server's address from a config file in its
installation folder, usually `login.cfg` (some client setups use
`swg2uu_login.cfg` instead):

```ini
[ClientGame]
loginServerAddress0=192.168.1.10
loginServerPort0=44990
```

Use the server's address, or `127.0.0.1` when the client runs on the same
machine as the servers. Then log in with an account created by
`setup.sh add-account` ([[Database]]).

## After logging in

- The galaxy shows as **"Loading..."** until the tutorial and Tatooine zones
  have started (a minute or two after `./server.sh start`).
- New characters start in the **tutorial**, which can be played through or
  skipped to go straight to Tatooine.
- Characters can be deleted and created from the character screen; each
  account has 2 slots by default ([[Database]] has how to raise it).

## Testing

What has been tested with a client, and what still needs testing, is in the
[Test plan](https://github.com/swganh/mmoserver/blob/master/docs/TEST_PLAN.md).
Reports of what works and what does not are welcome ([[Contributing]]).
