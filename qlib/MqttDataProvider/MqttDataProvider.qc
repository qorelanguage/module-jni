# -*- mode: qore; indent-tabs-mode: nil -*-
#! Qore MqttDataProvider module definition

/** MqttDataProvider.qc Copyright 2022 Qore Technologies, s.r.o.

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

%module-cmd(jni) import org.eclipse.paho.client.mqttv3.MqttClient
%module-cmd(jni) import org.eclipse.paho.client.mqttv3.MqttConnectOptions
%module-cmd(jni) import org.eclipse.paho.client.mqttv3.persist.MemoryPersistence
%module-cmd(jni) import org.eclipse.paho.client.mqttv3.IMqttMessageListener
%module-cmd(jni) import org.eclipse.paho.client.mqttv3.MqttMessage

%try-module json >= 1.5
%define NoJson
%endtry

#! contains all public definitions in the MqttDataProvider module
public namespace MqttDataProvider {
#! MQTT application name
public const AppName = "MQTT";

#! Event- and message-based data provider for MQTT client-side events
/**
*/
public class MqttDataProvider inherits DataProvider::AbstractDataProvider, DataProvider::DelayedObservable {
    public {
        #! Provider info
        const ProviderInfo = <DataProviderInfo>{
            "supports_messages": MSG_Async,
            "supports_observable": True,
            "constructor_options": ConstructorOptions,
            "desc": "Parent data provider for MQTT messaging functionality",
        };

        #! Constructor options
        const ConstructorOptions = {
            "auto_reconnect": <DataProviderOptionInfo>{
                "display_name": "Auto Reconnect",
                "short_desc": "Sets whether the client will automatically attempt to reconnect",
                "type": AbstractDataProviderTypeMap."bool",
                "desc": "Sets whether the client will automatically attempt to reconnect to the server if the "
                    "connection is lost\n"
                    "- If set to `False`, the client will not attempt to automatically reconnect to the server in "
                    "the event that the connection is lost\n"
                    "- If set to `True`, in the event that the connection is lost, the client will attempt to "
                        "reconnect to the server. It will initially wait 1 second before it attempts to reconnect, "
                        "for every failed reconnect attempt, the delay will double until it is at 2 minutes at "
                        "which point the delay will stay at 2 minutes",
            },
            "clean_session": <DataProviderOptionInfo>{
                "display_name": "Start With Clean Session",
                "short_desc": "Sets whether the client and server should remember state across restarts and "
                    "reconnects",
                "type": AbstractDataProviderTypeMap."bool",
                "desc": "Sets whether the client and server should remember state across restarts and reconnects.\n"
                    "- If set to `False` both the client and server will maintain state across restarts of the \n"
                    "client, the server and the connection. As state is maintained:\n"
                    "  - Message delivery will be reliable meeting the specified QOS even if the client, server or \n"
                    "connection are restarted.\n"
                    "  - The server will treat a subscription as durable.\n"
                    "- If set to `True` the client and server will not maintain state across restarts of the client, "
                    "the server or the connection. This means:\n"
                    "  - Message delivery to the specified QOS cannot be maintained if the client, server or \n"
                    "connection are restarted\n"
                    "  - The server will treat a subscription as non-durable",
                "default_value": True,
            },
            "client_id": <DataProviderOptionInfo>{
                "display_name": "Client ID",
                "short_desc": "The MQTT client ID",
                "type": AbstractDataProviderTypeMap."string",
                "desc": "The MQTT client ID",
                "default_value": "QoreMqttClient",
            },
            "connect_timeout": <DataProviderOptionInfo>{
                "display_name": "Connect Timeout",
                "short_desc": "The connect timeout to the MQTT server in seconds",
                "type": AbstractDataProviderTypeMap."int",
                "desc": "The connect timeout to the MQTT server in seconds",
                "default_value": 30,
            },
            "keep_alive_interval": <DataProviderOptionInfo>{
                "display_name": "Keep-Alive Interval",
                "short_desc": "Sets the keep-alive interval in seconds",
                "type": AbstractDataProviderTypeMap."int",
                "desc": "Sets the \"keep alive\" interval. This value, measured in seconds, defines the maximum \n"
                    "interval between messages sent or received. It enables the client to detect if the server is \n"
                    "no longer available, without having to wait for the TCP/IP timeout. The client will ensure \n"
                    "that at least one message travels across the network within each keep alive period. In the \n"
                    "absence of a data-related message during the time period, the client sends a very small "
                    "\"ping\" message, which the server will acknowledge. A value of `0` disables keepalive "
                    "processing in the client",
                "default_value": 60,
            },
            "max_in_flight": <DataProviderOptionInfo>{
                "display_name": "Max In Flight",
                "short_desc": "Set the max inflight value",
                "type": AbstractDataProviderTypeMap."int",
                "desc": "Sets the \"max inflight\" value; please increase this value in a high traffic environment",
                "default_value": 10,
            },
            "password": <DataProviderOptionInfo>{
                "display_name": "Password",
                "short_desc": "The password to use",
                "type": AbstractDataProviderTypeMap."string",
                "desc": "The password to use for the connection; if any",
                "sensitive": True,
            },
            "subscription_qos": <DataProviderOptionInfo>{
                "display_name": "Subscription QoS",
                "short_desc": "The maximum quality of service at which to subscribe",
                "type": AbstractDataProviderTypeMap."int",
                "desc": "The maximum quality of service at which to subscribe. Messages published at a lower "
                    "quality of service will be received at the published QoS. Messages published at a higher "
                    "quality of service will be received using the QoS specified on the subscribe",
                "required": True,
                "default_value": 2,
            },
            "subscription_topic": <DataProviderOptionInfo>{
                "display_name": "Topic",
                "short_desc": "The topic to use when subscribing",
                "type": AbstractDataProviderTypeMap."string",
                "desc": "The topic to use when subscribing to messages",
                "required": True,
                "default_value": "#",
            },
            "url": <DataProviderOptionInfo>{
                "display_name": "URL",
                "short_desc": "The URL to the MQTT server",
                "type": AbstractDataProviderTypeMap."string",
                "desc": "The URL to the MQTT server; `tcp://` for unencrypted connections, `ssl://` for encrypted "
                    "connections",
                "required": True,
            },
            "username": <DataProviderOptionInfo>{
                "display_name": "Username",
                "short_desc": "The username to use",
                "type": AbstractDataProviderTypeMap."string",
                "desc": "The username to use for the connection; if any",
            },
        };

        #! MQTT message event constant
        const EVENT_MQTT_MESSAGE = "mqtt-message-event";

        #! MQTT message type
        /** Publishes an MQTT message to the given topic
        */
        const MESSAGE_MQTT_MESSAGE = "message";
    }

    private {
        #! The MQTT client itself
        MqttClient client;

        #! Connection options
        MqttConnectOptions opts;

        #! Subscription topic filter
        string topic;

        #! Subscription QoS
        int qos;
    }

    #! Creates the object from constructor options
    constructor(*hash<auto> options) {
        hash<auto> copts = checkOptions("CONSTRUCTOR-ERROR", ConstructorOptions, options);
        hash<auto> cinfo = getClient(copts);
        client = cinfo.client;
        opts = cinfo.opts;
        topic = copts.subscription_topic;
        qos = copts.subscription_qos;
    }

    destructor() {
        if (client.isConnected()) {
            client.disconnect();
            if (logger) {
                logger.debug("MQTT: disconnected from %y", client.getServerURI());
            }
        }
        delete client;
    }

    string getName() {
        return "mqtt";
    }

    #! Called when all observers have been added to the object
    /** This method is meant to trigger event generation
    */
    observersReady() {
        start();
    }

    #! Returns data provider static info
    hash<DataProvider::DataProviderInfo> getStaticInfoImpl() {
        return ProviderInfo;
    }

    #! Returns a client for the given options
    static hash<auto> getClient(hash<auto> copts) {
        # create the client object
        MemoryPersistence persistence();
        MqttClient client = new MqttClient(copts.url, copts.client_id, persistence);

        # set connection options
        MqttConnectOptions opts();
        opts.setConnectionTimeout(copts.connect_timeout);
        opts.setKeepAliveInterval(copts.keep_alive_interval);
        opts.setMaxInflight(copts.max_in_flight);
        opts.setCleanSession(copts.clean_session);

        if (exists copts.auto_reconnect) {
            opts.setAutomaticReconnect(copts.auto_reconnect);
        }
        if (exists copts.username) {
            opts.setUserName(copts.username);
        }
        if (exists copts.password) {
            opts.setPassword(new String(copts.password).toCharArray());
        }

        return {
            "client": client,
            "opts": opts,
        };
    }

    #! Connects and starts receiving messages
    private start() {
        connect();

        # get MessageListener interface class
        lang::Class ml = load_class("org/eclipse/paho/client/mqttv3/IMqttMessageListener");
        client.subscribe(topic, qos,
            cast<IMqttMessageListener>(implement_interface(new QoreInvocationHandler(\messageReceived()), ml)));
        if (logger) {
            logger.debug("MQTT: listening on topic: %y", topic);
        }
    }

    #! Connects to the server if necessary
    private connect() {
        if (client.isConnected()) {
            return;
        }
        if (logger) {
            logger.debug("MQTT: connecting to server %y", client.getServerURI());
        }
        client.connect(opts);
        if (logger) {
            logger.debug("MQTT: connected to server %y", client.getServerURI());
        }
    }

    #! Qore callback for <tt>IMqttMessageListener.messageArrived(java.lang.String topic, MqttMessage message)</tt>
    private messageReceived(Method method, *list<auto> args) {
        MqttMessage message = args[1];
        hash<auto> msghash = {
            "id": message.getId(),
            "qos": message.getQos(),
            "payload": message.getPayload(),
            "duplicate": message.getDuplicate(),
            "retained": message.getRetained(),
        };
        if (logger) {
            logger.debug("MQTT message received: %y", msghash);
        }
        notifyObservers(EVENT_MQTT_MESSAGE, msghash);
    }

    #! Returns a hash of all supported event types
    /** @return a hash of all supported event types

        @note only called if the provider supports the observer pattern / event API
    */
    private hash<string, hash<DataProviderMessageInfo>> getEventTypesImpl() {
        return {
            EVENT_MQTT_MESSAGE: <DataProviderMessageInfo>{
                "desc": "This event is raised when an incoming MQTT message matching the subscription topic arrives",
                "type": new MqttEventDataType(),
            },
        };
    }

    #! Returns the description of an outbound message, if any
    /** @return the message type for this provider

        @note only called if the provider supports sending messages
    */
    private hash<string, hash<DataProviderMessageInfo>> getMessageTypesImpl() {
        return {
            MESSAGE_MQTT_MESSAGE: <DataProviderMessageInfo>{
                "desc": "An outgoing MQTT message that will be published according to the message contents",
                "type": new MqttMessageDataType(),
            },
        };
    }

    #! Sends a message from message-capable data providers
    /**
        @param msg the message to send
        @param send_message_options the options for sending the message
    */
    private sendMessageImpl(string message_id, auto msg, *hash<auto> send_message_options) {
        try {
            switch (message_id) {
                case MESSAGE_MQTT_MESSAGE: {
                    {
                        int pt = msg.payload.typeCode();
                        if (pt != NT_STRING && pt != NT_BINARY) {
%ifndef NoJson
                            msg.payload = make_json(msg.payload);
%else
                            throw "MESSAGE-SERIALIZATION-ERROR", sprintf("cannot serialize a message of type %y to "
                                "JSON without the json module", msg.payload.fullType());
%endif
                        }
                    }
                    softbinary payload = msg.payload;
                    connect();
                    client.publish(msg.topic, payload, msg.qos ?? 2, msg.retained ?? False);
                    if (logger) {
                        logger.debug("MQTT publish topic: %y qos: %y payload: %y retained: %y", msg.topic, msg.qos,
                            msg.payload, msg.retained);
                    }
                    break;
                }

                default:
                    throw "UNSUPPORTED-MESSAGE", sprintf("message_id %y is unsupported; supported messages: %y",
                        keys getMessageTypesImpl());
            }
        } catch (hash<ExceptionInfo> ex) {
            if (logger) {
                logger.error("error sending MQTT message: %s", get_exception_string(ex));
            }
        }
    }
}

#! Class that encapsulates the client and connection options
public class MqttConnectionWrapper {
    public {
        MqttClient client;
        MqttConnectOptions opts;
    }

    #! Creates the object from the arguments
    constructor(MqttClient client, MqttConnectOptions opts) {
        self.client = client;
        self.opts = opts;
    }

    #! Redirects method calls to the client
    auto methodGate(string method, ...) {
        return call_object_method_args(client, method, argv);
    }
}

public class MqttEventDataType inherits HashDataType {
    private {
        #! Field descriptions
        const Fields = {
            "id": {
                "type": IntType,
                "desc": "The MQTT id of the message",
            },
            "qos": {
                "type": IntType,
                "desc": "The quality of service value for this message",
                "allowed_values": (0, 1, 2),
            },
            "payload": {
                "type": DataType,
                "desc": "The payload / message body",
            },
            "duplicate": {
                "type": BoolType,
                "desc": "If this message might be a duplicate of one which has already been received",
            },
            "retained": {
                "type": BoolType,
                "desc": "If this message was retained by the server",
            },
        };
    }

    #! Creates the object
    constructor() {
        map addField(new QoreDataField($1.key, $1.value.desc, $1.value.type, $1.value.default_value,
            $1.value.allowed_values)), Fields.pairIterator();
    }
}

public class MqttMessageDataType inherits HashDataType {
    private {
        #! Field descriptions
        const Fields = {
            "topic": {
                "type": StringType,
                "desc": "The message topic",
            },
            "qos": {
                "type": IntType,
                "desc": "The quality of service value for this message",
                "default_value": 2,
                "allowed_values": (0, 1, 2),
            },
            "payload": {
                "type": AutoType,
                "desc": "The payload / message body; a string or binary type is sent as-is, all others are "
                    "serialized with JSON and sent as a JSON string",
            },
            "retained": {
                "type": BoolType,
                "desc": "If this message should be retained by the server",
                "default_value": False,
            },
        };
    }

    #! Creates the object
    constructor() {
        map addField(new QoreDataField($1.key, $1.value.desc, $1.value.type, $1.value.default_value,
            $1.value.allowed_values)), Fields.pairIterator();
    }
}
}
