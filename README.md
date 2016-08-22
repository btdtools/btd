# btd
### A BibTex daemon
`btd` is a daemon that serves bibtex files possibly with attached documents. At
the moment it is just a proof of concept and not functional at all. In the
future it will have several frontends and advanced import features.

### Documentation
#### Installation
To compile you can run `make`. Note that this requires the development headers
of [sqlite][sqlite]. On debian (derivatives) you can just install
`libsqlite3-dev`.

To create manpage can run `make man` after installing `help2man`.

Installation can be done by running `# make install`. You can change the
installation location by changing the `PREFIX` variable in the `Makefile`.

#### Configuration
Almost all configuration is done via the config file. The program looks for a
config file in `$XDG_CONFIG_HOME/btd/config` and in all the components of
`$XDG_CONFIG_DIRS/btd/config`. The config can also be manually specified by
using the command line argument.

An example configuration file with all the values is included as
`config.example`.

#### Data storage
The program stores an [sqlite][sqlite] database containing the metadata and an
optional file system storing the attached documents in a directory. The program
checks whether the directory `$XDG_DATA_HOME/btd` or one of the components of
`$XDG_DATA_DIRS/btd` exists and will store the data in the corresponding match.
When no data directory exists `btd` will store the data in
`$XDG_DATA_HOME/btd`.

### LICENSE
MIT Licence, see `LICENSE` file

### Hacking
We try to adhere to Linux Kernel Coding style.

### Author(s)
Mart Lubbers

#### Notes
When the XDG environment variables are not declared we fall back to the
standards described [here][xdgspec].

- `$XDG_CONFIG_HOME`: `~/.config`
- `$XDG_CONFIG_DIRS`: `/etc/xdg`
- `$XDG_DATA_HOME`: `~/.local/share`
- `$XDG_DATA_DIRS`: `/usr/local/share:/usr/share`

[sqlite]: https://sqlite.org/ "SQLite"
[xdgspec]: https://specifications.freedesktop.org/basedir-spec/basedir-spec-latest.html "Latest XDG spec"
