# -*- mode: qore; indent-tabs-mode: nil -*-
#! Qore MqttDataProvider module definition

/** MqttConnection.qc Copyright 2022 Qore Technologies, s.r.o.

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
*/

#! contains all public definitions in the MqttDataProvider module
public namespace MqttDataProvider {
#! class for Microsoft Common Data Service REST connections; returns @ref CdsRestClient objects
/** supports the following options:
    - TBD
*/
public class MqttConnection inherits ConnectionProvider::AbstractConnectionWithInfo {
    public {
        #! Connection entry info
        const ConnectionScheme = <ConnectionSchemeInfo>{
            "display_name": "MQTT Connection",
            "short_desc": "A connection to an MQTT server",
            "desc": "A connection to an MQTT server",
            "cls": Class::forName("MqttConnection"),
            "options": cast<hash<string, hash<ConnectionOptionInfo>>>(
                map {
                    $1.key: cast<hash<ConnectionOptionInfo>>($1.value - ("type", "required") + {
                        "type": MqttConnection::getType($1.value),
                    })
                }, MqttDataProvider::ConstructorOptions.pairIterator(), $1.key !~ /^subscription_/
            ),
        };
    }

    #! Creates the object from the arguments
    constructor(string name, string desc, string url, hash<auto> attr = {}, hash<auto> options = {},
            *LoggerInterface logger) : AbstractConnectionWithInfo(name, desc, url, attr, options, logger) {
    }

    #! returns the string type name for the connection object
    string getType() {
        return "mqtt";
    }

    #! Returns @ref True, as this connection class returns a data provider with the @ref getDataProvider() method
    /** @return @ref True, as this connection class returns a data provider with the @ref getDataProvider() method

        @see @ref getDataProvider()
    */
    bool hasDataProvider() {
        return True;
    }

    #! Returns a @ref MqttDataProvider object for this connection
    /** @param constructor_options any additional constructor options for the data provider

        @return a data provider object for this connection, if supported
    */
    AbstractDataProvider getDataProvider(*hash<auto> constructor_options) {
        return new MqttDataProvider(orig_opts + constructor_options);
    }

    #! Returns the string type for the first data provider type
    static string getType(hash<DataProviderOptionInfo> opt) {
        string type = opt.type[0].getName();
        if (!opt.required && type[0] != "*") {
            splice type, 0, 0, "*";
        }
        return type;
    }

    #! Returns the ConnectionSchemeInfo hash for this object
    private hash<ConnectionSchemeInfo> getConnectionSchemeInfoImpl() {
        return ConnectionScheme;
    }

    #! Returns a hash of data provider constructor option info, if applicable
    /** @return a hash of data provider constructor option info for the primary data provider, if applicable
    */
    private *hash<string, hash<DataProviderOptionInfo>> getDataProviderConstructorOptionsImpl() {
        return MqttDataProvider::ConstructorOptions;
    }

    #! Override to set child data provider capabilities once per child class
    private setChildCapabilities() {
        children_can_support_observers = True;
        children_can_support_messages = True;
    }

    #! this method must return the connection object corresponding to the object's configuration
    /** @param connect if @ref True "True" then the connection should be returned connected (and if possible a
        round-trip "ping" of the server should be made)
        @param rtopts options to override connection options

        @return the connection object
    */
    private MqttConnectionWrapper getImpl(bool connect = True, *hash<auto> rtopts) {
        hash<auto> copts = MqttDataProvider::getClient(orig_opts + rtopts);
        if (connect) {
            copts.client.connect(copts.opts);
        }
        return new MqttConnectionWrapper(copts.client, copts.wrapper);
    }
}
}

