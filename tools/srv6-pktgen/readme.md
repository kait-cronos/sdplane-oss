### Topoloy Overview
![Topology](./topology.dio.png)

### Configuration
```sh
# ++++++ in sdplane dev machine +++++
$ make sdplane-up
$ make conf
# ++++++ in tester machine ++++++ 
$ make ns-up NIC=eno2
$ make ns-down
```

