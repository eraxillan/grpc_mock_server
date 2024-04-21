# gRPC mock server library

This is a written in C++/Python/CMake gRPC mock server.  
It allows to overwrite responses data, partially or fully, using mock data from configuration file.  
By default, all requests are simply forwarded to the specified remote gPRC server, so this program acts as a proxy.  
Using special offline mode, one can fully control responses

## Building
### Common OS-independent steps
* Install vcpkg using [this guide](https://github.com/microsoft/vcpkg/tree/master?tab=readme-ov-file#getting-started)
* `git clone https://github.com/eraxillan/grpc_mock_server`
* `git submodule update --init`
* `cd grpc_mock_server`

### Windows 11

* Install Visual Studio Community 2022
* Execute the command in Windows terminal
```
"C:\WINDOWS\system32\cmd.exe" /c "%SYSTEMROOT%\System32\chcp.com 65001 >NUL && "C:\PROGRAM FILES\MICROSOFT VISUAL STUDIO\2022\COMMUNITY\COMMON7\IDE\COMMONEXTENSIONS\MICROSOFT\CMAKE\CMake\bin\cmake.exe"  -G "Ninja"  -DCMAKE_C_COMPILER:STRING="cl.exe" -DCMAKE_CXX_COMPILER:STRING="cl.exe" -DCMAKE_TOOLCHAIN_FILE:STRING="<path/to/vcpkg/root>/vcpkg.cmake" -DPROTO_API_PATH:STRING="<path/to/proto/directory>" -DCMAKE_BUILD_TYPE:STRING="Debug" -DCMAKE_MAKE_PROGRAM="C:\PROGRAM FILES\MICROSOFT VISUAL STUDIO\2022\COMMUNITY\COMMON7\IDE\COMMONEXTENSIONS\MICROSOFT\CMAKE\Ninja\ninja.exe" "grpc_mock_server" 2>&1"
```
* OR
* Copy file `grpc_mock_server/CMakeUserPresets_template.json` to `grpc_mock_server/CMakeUserPresets.txt`
* Open your favorite text editor and replace CMAKE_TOOLCHAIN_FILE variable value to your actual vcpkg installation root
* Open the Visual Studio
* Click "Continue without code", "Open", "CMake"
* Select the grpc_mock_server/CMakeLists.txt
* Wait for CMake configuration step completion (it can take a several minutes depending on your Internet connection)
* Press F6 to build project
* Select grpc-mock-server-executable.exe target
* Press F5 to launch it (program will show error with valid configuration file)

### Debian-based Linux
* NOTE: tested only on Ubuntu in WSL, but should work at any Linux, only package names will differ
* Execute the following commands ()
```
sudo apt-get install curl cmake git build-essential tar curl zip unzip ninja-build autoconf automake autoconf-archive libtool pkg-config
cmake -B out-linux -S . -G "Ninja" -DCMAKE_BUILD_TYPE="Debug" "-DCMAKE_TOOLCHAIN_FILE=<path/to/vcpkg/root>/scripts/buildsystems/vcpkg.cmake" "-DCMAKE_MAKE_PROGRAM=ninja" "-DPROTO_API_PATH=<path/to/proto/directory>"
cmake --build out-linux
```

# macOS
TODO: not fully supported yet due to compiler incompatibility issue
* Generate Xcode project
```
brew install cmake
cmake -B out-macos-xcode -S . -G "Xcode" -DCMAKE_BUILD_TYPE:STRING="Debug" -DCMAKE_TOOLCHAIN_FILE=<path/to/vcpkg/root>/scripts/buildsystems/vcpkg.cmake -DCMAKE_MAKE_PROGRAM=xcodebuild "-DPROTO_API_PATH=<path/to/proto/directory>"
```
* Direct build from command-line
```
brew install cmake ninja autoconf automake autoconf-archive
cmake -B out-macos -S . -G "Ninja" -DCMAKE_BUILD_TYPE:STRING="Debug" -DCMAKE_TOOLCHAIN_FILE=<path/to/vcpkg/root>/scripts/buildsystems/vcpkg.cmake -DCMAKE_MAKE_PROGRAM=ninja "-DPROTO_API_PATH=<path/to/proto/directory>"
cmake --build out-macos
```

## Configurarion file format
Which requests to override and what data server will return should be defined in XML-format configuration file.  

```
<remote_host_url value="grpc.your-company-host.com"/>
```
Remote host URL to redirect requests without substitution to.  
Not used in offline mode
```
<remote_host_port value="443"/>
```
Remote host port to redirect requests without substitution to.  
Not used in offline mode
```
<local_host_port value="50051"/>
```
gRPC server local port value, must not be blocked by firewall application or network configuration rules
```
<is_offline_mode_enabled value="true"/>
```
Offline mode switch.  
If enabled, all responses without substitution will return "not implemented" error instead of redirection to remote server
```
<dataset name="fixed_price_1234">
    <package name="grpc_api_package">
        <service name="order_service">
            <method name="ListOrders" >
                <full path="path/to/list_orders_full_response.txt" />
                <partial path="path/to/list_orders_partial_response.txt" />
            </method>
        </service>
    </package>
</dataset>
```

`list_orders_full_response.txt` - order data in JSON format
`list_orders_full_response.txt` - order fields data in custom Pascal-like language, e.g.
```
orders[].price := 1234.4321
```
where `order[]` means "array-typed field order", `price` means "price field", `:=` - "replace value with", and `1234.4321` - "replace original value with specified one"  
NOTE: only one of `full` or `partial` node can be speicifed

## Modifying your gRPC client
Example for Android
* Channel setup
```
val channelBuilder = AndroidChannelBuilder
    .forAddress("<gRPC mock server IP address>", <gRPC mock server local port>)
    .context(androidContext())
    .useTransportSecurity()

// Hack to obtain an `okHttpChannelBuilder` object
val delegateMethod = channelBuilder::class.java.getDeclaredMethod("delegate")
delegateMethod.isAccessible = true
val okHttpChannelBuilder: OkHttpChannelBuilder = delegateMethod.invoke(channelBuilder) as OkHttpChannelBuilder

// Trust the mock gRPC server certificate (need a less hacky way to do this, yep)
val trustAllCertsManager = object : X509TrustManager {
    override fun checkClientTrusted(chain: Array<out X509Certificate>?, authType: String?) {
    }

    override fun checkServerTrusted(chain: Array<out X509Certificate>?, authType: String?) {
    }

    override fun getAcceptedIssuers(): Array<X509Certificate> {
        return emptyArray()
    }
}

val sslContext = SSLContext.getInstance("SSL");
sslContext.init(null, arrayOf(trustAllCertsManager), SecureRandom())
okHttpChannelBuilder.sslSocketFactory(sslContext.socketFactory)
okHttpChannelBuilder.hostnameVerifier { hostname, session -> true }

channelBuilder.build()
```
* Request setup
```
val header = io.grpc.Metadata()
val key = io.grpc.Metadata.Key.of("gms_dataset", io.grpc.Metadata.ASCII_STRING_MARSHALLER)
header.put(key, "fixed_price_1234")
return order_service.withInterceptors(MetadataUtils.newAttachHeadersInterceptor(header)).listOrders(request)
```

## Running
One can specify concrete configurarion file using `-c` option:  
`grpc_mock_server_executable -c config.xml`
