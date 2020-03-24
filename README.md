pkg is a simple, minimal dependency manager based on Git.

It clones your cycle-free dependency tree recursively and creates a `CMakeLists.txt` file with all dependencies in topological order.

Example `.pkg` file:

    [fmt]
      url=git@github.com:motis-project/fmt.git
      branch=master
      commit=3eca62d66c7c9a2ca97dc8381299d911978b9fb2
