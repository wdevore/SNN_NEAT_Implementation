# SNN_NEAT_Implementation
A C++17 Raylib implementation of NEAT for neural networks. It is several projects building on top of the base algorithm.

## Debugging and Analysis

A key part of working with genetic algorithms is comparing the output of different runs to identify divergences.

### Command-Line File Comparison

For comparing files on the command line, the classic and most powerful tool is `diff`. It's available by default on virtually all Linux, macOS, and other Unix-like systems (including Git Bash on Windows). While VS Code's visual diff is great, the command-line `diff` is extremely fast and scriptable.

#### Unified Format (`-u`)

This is the industry-standard format. It shows differences as a single block of text with lines prefixed by `+` (added), `-` (removed), or a space (unchanged context).

**How to use it:**

```bash
diff -u base_log.txt raylib_log.txt
```

**How to read the output:**

```diff
... (context lines)
 DEBUG: Mutate Only. Roll: 0.04 < 0.05
 DEBUG: Mutate Parent Choice: orgnum=12
-DEBUG: Mutate Add Link. Roll: 0.04 < 0.05
-DEBUG: Mutate Add Link result: SUCCESS
+DEBUG: Mutate Add Link. Roll: 0.06 < 0.05
+DEBUG: Mutate Add Link result: FAIL
 DEBUG: Mutate link weights...
... (context lines)
```

* Lines starting with - exist only in the first file (base_log.txt).
* Lines starting with + exist only in the second file (raylib_log.txt).
* Lines starting with a space are identical and are shown for context.

**Pro Tip:** Your log files are very long. To make them easier to read, you can "pipe" the output of `diff` into a pager like `less`:

```bash
diff -u base_log.txt raylib_log.txt | less
```

This will let you scroll through the differences using your arrow keys, Page Up/Down, and search by typing `/` followed by your search term. Press `q` to quit.

### Enhancements and Alternatives

1.  **`colordiff`**: This is a wrapper around `diff` that adds color to the output, making it significantly easier to read at a glance. On most Linux systems, you can install it with your package manager (e.g., `sudo apt-get install colordiff`). You use it exactly like `diff`:
    ```bash
    colordiff -u base_log.txt raylib_log.txt | less -R
    ```
    (The `-R` flag for `less` ensures the colors are displayed correctly).

2.  **Useful `diff` Flags**:
    *   `-w` or `--ignore-all-space`: This is very useful for ignoring differences in indentation or trailing spaces.
    *   `-i` or `--ignore-case`: Ignores whether characters are uppercase or lowercase.
    *   `-y` or `--side-by-side`: Displays the differences in two columns.
        * `colordiff` `-y` **baselib.log** **raylib.log** `>` **diff.log**

## RAMDisk
```sh
> sudo mkdir -p /media/RAMDisk
> sudo mount -t tmpfs -o size=4096M tmpfs /media/RAMDisk/
```