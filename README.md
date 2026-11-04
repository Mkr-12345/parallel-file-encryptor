# Parallel File Encryptor

A C++17 command-line tool that encrypts and decrypts files in batch using multiple worker processes. It is built from modular components for file I/O, task management, and process execution.

> **Security note:** The cipher is a byte-wise **Caesar cipher** (shift mod 256). It is a deliberately simple algorithm used to demonstrate the parallel processing pipeline. It is **not secure** and must not be used to protect real data. The `Cipher` module is isolated so it can be replaced with AES (e.g. via OpenSSL).

## Features

- **Parallel batch processing:** each file is handled by its own `fork()`ed worker, with at most `N` workers alive at once (`--jobs`).
- **Task queue + process executor:** `ProcessManagement` queues tasks, spawns workers, and reaps them with `wait()`.
- **Recursive directory driver:** walks directory trees with the C++ `<filesystem>` library and enqueues every eligible file.
- **`.env`-based key management:** the key is read from `ENCRYPTION_KEY` in a `.env` file and is never passed on the command line.
- **Single-file CLI mode:** pass a file instead of a directory to process it directly in the current process.
- **Process-safe logging:** workers append to a shared log file (`logs/fileenc.log`).
- **Binary-safe, streaming I/O:** 64 KiB chunks, so large files are fine.

## Project layout

```
include/            headers
  Task.hpp            Task + Mode (encrypt/decrypt)
  Cipher.hpp          CaesarCipher
  EnvLoader.hpp       .env parser / key loading
  FileIO.hpp          chunked file transform + output naming
  DirectoryScanner.hpp  recursive directory -> tasks
  ProcessManagement.hpp task queue + fork-based worker pool
  Logger.hpp          process-safe logger
src/                implementations + main.cpp (CLI)
tests/run_tests.sh  round-trip and error-handling tests
Makefile
```

## Build

Requires a C++17 compiler and `make` on Linux or macOS (uses POSIX `fork`).

```bash
make          # builds ./fileenc
make test     # runs tests/run_tests.sh
make clean
```

## Usage

```bash
cp .env.example .env        # then edit ENCRYPTION_KEY

# single file
./fileenc encrypt notes.txt            # -> notes.txt.enc
./fileenc decrypt notes.txt.enc        # -> notes.txt.dec

# whole directory, recursively, 8 parallel workers
./fileenc encrypt ./my_folder -j 8
./fileenc decrypt ./my_folder -j 8
```

| Option | Description | Default |
|---|---|---|
| `-j, --jobs N` | max parallel worker processes | CPU cores |
| `-e, --env PATH` | path to the `.env` file | `.env` |
| `-l, --log PATH` | log file | `logs/fileenc.log` |

Encrypt writes `<name>.enc` and skips files already ending in `.enc`. Decrypt processes only `.enc` files and writes `<name>.dec`. Originals are never modified or deleted.

Exit codes: `0` success, `1` usage or setup error, `2` some files failed.

## How it works

```
main (CLI)
  |-- EnvLoader        reads ENCRYPTION_KEY from .env
  |-- DirectoryScanner recursive walk -> Task{in, out, mode}
  |-- ProcessManagement
  |     queue --> fork() --> child: FileIO::transformFile + CaesarCipher
  |                          parent: wait() and tally success/failure
  `-- Logger           append-only log shared by all processes
```

## License

MIT
