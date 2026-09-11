/*  NodeSet2Importer.java Copyright 2026 Qore Technologies, s.r.o.

    Generic offline OPC UA NodeSet2 importer (Epic A Phase 4).

    Parses an OPC UA NodeSet2 XML document into the same schema snapshot shape produced by
    SchemaResolver, using the JDK's built-in XML parser (no external dependency). File retrieval is
    done by the caller (Qore-side, via FileLocationHandler); this class parses the XML content.

    Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
    associated documentation files (the "Software"), to deal in the Software without restriction. THE
    SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND.
*/

package org.qore.dataprovider.opcua;

import java.io.StringReader;
import java.util.ArrayDeque;
import java.util.ArrayList;
import java.util.Deque;
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;

import javax.xml.XMLConstants;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;

import org.qore.jni.Hash;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.InputSource;

/** Imports an OPC UA NodeSet2 XML document into a generic schema snapshot. */
public class NodeSet2Importer {
    /** The standard OPC UA namespace, always index 0. */
    private static final String UA_NAMESPACE = "http://opcfoundation.org/UA/";

    /** Maximum browse path depth when walking a node's parent chain (cycle/runaway guard). */
    private static final int MAX_PATH_DEPTH = 24;

    /** The NodeSet2 elements that declare an address-space node (any other element is metadata). */
    private static final Set<String> NODE_ELEMENTS = Set.of(
        "UAObject", "UAObjectType", "UAVariable", "UAVariableType", "UAMethod", "UADataType",
        "UAReferenceType", "UAView");

    /**
     * The reference types that place a node in the browse hierarchy, both as standard NodeIds and under
     * their conventional alias names (a NodeSet2 {@code ReferenceType} attribute may use either).
     *
     * <p>{@code HasSubtype} is deliberately absent: it is hierarchical, but it relates a type to its
     * supertype rather than placing a node inside a containing node, so it must not contribute a browse
     * path segment (a type defined in a model is browsed at the top of that model, not under
     * {@code BaseObjectType}).
     */
    private static final Set<String> CONTAINMENT_REFERENCES = Set.of(
        "i=35", "Organizes",
        "i=46", "HasProperty",
        "i=47", "HasComponent",
        "i=49", "HasOrderedComponent",
        "i=17604", "HasAddIn");

    /**
     * Standard address-space nodes that a model's own nodes commonly hang from, mapped to the browse
     * path segment they contribute.
     *
     * <p>The Root and Objects folders map to the empty segment because the snapshot browse path is
     * rooted at the Objects folder, exactly as the live {@link SchemaResolver} walk is. Every other
     * standard parent contributes its own segment so that same-named nodes under different standard
     * folders (the binary and XML type dictionaries both named {@code Measurements}, for example) stay
     * distinct.
     */
    private static final Map<String, String> STANDARD_PARENTS = Map.ofEntries(
        Map.entry("i=84", ""),                  // Root
        Map.entry("i=85", ""),                  // Objects: the browse root of a snapshot
        Map.entry("i=86", "0:Types"),
        Map.entry("i=87", "0:Views"),
        Map.entry("i=88", "0:ObjectTypes"),
        Map.entry("i=89", "0:VariableTypes"),
        Map.entry("i=90", "0:DataTypes"),
        Map.entry("i=91", "0:ReferenceTypes"),
        Map.entry("i=92", "0:XmlSchema"),
        Map.entry("i=93", "0:OPC Binary"),
        Map.entry("i=2253", "0:Server"));

    /** The browse names of the method properties that carry a method's argument metadata. */
    private static final String INPUT_ARGUMENTS = "0:InputArguments";
    private static final String OUTPUT_ARGUMENTS = "0:OutputArguments";

    /** A NodeSet2 node element with its identity and its resolved place in the browse hierarchy. */
    private static class NodeSetNode {
        /** The node's XML element. */
        final Element element;

        /** The local name of the node's element (e.g. \c UAVariable). */
        final String tag;

        /** The node's NodeId, exactly as written in the document. */
        final String nodeId;

        /** The node's namespace-qualified browse name (always \c N:Name, index 0 made explicit). */
        final String browseName;

        /** The NodeId of the containing node, or null when the node is at the top of the model. */
        String parentNodeId;

        NodeSetNode(Element element, String tag, String nodeId, String browseName) {
            this.element = element;
            this.tag = tag;
            this.nodeId = nodeId;
            this.browseName = browseName;
        }
    }

    /**
     * Parses a NodeSet2 XML document into a schema snapshot (same shape as SchemaResolver).
     *
     * <p>Browse paths are reconstructed from each node's {@code ParentNodeId} (or, when absent, from its
     * inverse containment reference), so nodes that share a browse name under different parents remain
     * distinct and the model's hierarchy survives the import. A method's {@code InputArguments} and
     * {@code OutputArguments} properties are folded into the method endpoint's argument lists rather
     * than imported as variable endpoints of their own.
     *
     * @param xml the NodeSet2 XML content
     * @return the snapshot as an org.qore.jni.Hash; \c source is "imported" and it additionally carries
     *     a \c required_models list (the namespace URIs the model depends on)
     * @throws Exception on a parse failure
     */
    public static Hash parse(String xml) throws Exception {
        DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
        // NodeSet2 declares a default namespace; local-name lookups are simplest with awareness off
        factory.setNamespaceAware(false);
        // harden the parser against XXE: enable secure processing, forbid DOCTYPE entirely, and deny
        // all external DTD/schema resolution and entity expansion
        factory.setFeature(XMLConstants.FEATURE_SECURE_PROCESSING, true);
        factory.setFeature("http://apache.org/xml/features/disallow-doctype-decl", true);
        factory.setAttribute(XMLConstants.ACCESS_EXTERNAL_DTD, "");
        factory.setAttribute(XMLConstants.ACCESS_EXTERNAL_SCHEMA, "");
        factory.setExpandEntityReferences(false);
        DocumentBuilder builder = factory.newDocumentBuilder();
        Document doc = builder.parse(new InputSource(new StringReader(stripByteOrderMark(xml))));
        Element root = doc.getDocumentElement();
        if (root == null || !"UANodeSet".equals(localName(root.getTagName()))) {
            throw new IllegalArgumentException("the NodeSet2 root element must be UANodeSet");
        }

        // namespace table: index 0 is the standard UA namespace, then the declared NamespaceUris in order
        List<String> uris = new ArrayList<>();
        uris.add(UA_NAMESPACE);
        Element namespaceUris = firstChild(root, "NamespaceUris");
        if (namespaceUris != null) {
            for (Element uri : childElements(namespaceUris, "Uri")) {
                uris.add(uri.getTextContent().trim());
            }
        }

        Hash snapshot = new Hash();
        snapshot.put("opcua", AddressSpaceSchema.OPCUA_VERSION);
        snapshot.put("contract_version", SchemaResolver.CONTRACT_VERSION);
        snapshot.put("source", "imported");

        Hash namespaces = new Hash();
        for (int i = 0; i < uris.size(); ++i) {
            namespaces.put(String.valueOf(i), uris.get(i));
        }
        snapshot.put("namespaces", namespaces);

        // required model dependencies (Models/Model/RequiredModel ModelUri)
        List<Object> requiredModels = new ArrayList<>();
        Element models = firstChild(root, "Models");
        if (models != null) {
            for (Element model : childElements(models, "Model")) {
                for (Element required : childElements(model, "RequiredModel")) {
                    String modelUri = required.getAttribute("ModelUri");
                    if (!modelUri.isEmpty()) {
                        requiredModels.add(modelUri);
                    }
                }
            }
        }
        snapshot.put("required_models", requiredModels);

        // dependency check: required model namespaces not present in this model's namespace table
        // (the standard UA namespace is always considered present)
        Set<String> known = new HashSet<>(uris);
        known.add(UA_NAMESPACE);
        List<Object> missing = new ArrayList<>();
        for (Object modelUri : requiredModels) {
            if (!known.contains(String.valueOf(modelUri))) {
                missing.add(modelUri);
            }
        }
        snapshot.put("missing_dependencies", missing);

        // index every declared node so that browse paths can be built from the containment hierarchy;
        // reference types may be written as aliases, so the alias table is needed to recognize them
        Map<String, String> aliases = aliasTable(root);
        Map<String, NodeSetNode> nodes = indexNodes(root);
        resolveHierarchy(nodes, aliases);

        // a method's argument properties describe the method; they are not endpoints of their own
        Map<String, List<Object>> methodArguments = new HashMap<>();
        Set<String> argumentProperties = new HashSet<>();
        collectMethodArguments(nodes, aliases, methodArguments, argumentProperties);

        // endpoints: UAVariable -> variable, UAMethod -> method-call
        List<Object> endpoints = new ArrayList<>();
        Set<String> seen = new HashSet<>();
        for (NodeSetNode node : nodes.values()) {
            String kind;
            if ("UAVariable".equals(node.tag)) {
                if (argumentProperties.contains(node.nodeId)) {
                    continue;
                }
                kind = "variable";
            } else if ("UAMethod".equals(node.tag)) {
                kind = "method-call";
            } else {
                continue;
            }
            addEndpoint(endpoints, seen, uris, nodes, node, kind, aliases, methodArguments);
        }
        snapshot.put("endpoints", endpoints);

        return snapshot;
    }

    /**
     * Strips a leading UTF-8 byte order mark and any whitespace before the XML prolog.
     *
     * <p>Published NodeSet2 files are frequently serialized with a BOM, which the XML parser rejects
     * with "Content is not allowed in prolog"; tolerating it here means every caller does not have to.
     */
    private static String stripByteOrderMark(String xml) {
        if (xml == null) {
            return xml;
        }
        int start = 0;
        while (start < xml.length()
                && (xml.charAt(start) == '\uFEFF' || Character.isWhitespace(xml.charAt(start)))) {
            ++start;
        }
        return start > 0 ? xml.substring(start) : xml;
    }

    /** Returns the document's {@code Aliases} table (alias name -&gt; NodeId), empty when absent. */
    private static Map<String, String> aliasTable(Element root) {
        Map<String, String> rv = new HashMap<>();
        Element aliases = firstChild(root, "Aliases");
        if (aliases == null) {
            return rv;
        }
        for (Element alias : childElements(aliases, "Alias")) {
            String name = alias.getAttribute("Alias").trim();
            String nodeId = alias.getTextContent().trim();
            if (!name.isEmpty() && !nodeId.isEmpty()) {
                rv.put(name, nodeId);
            }
        }
        return rv;
    }

    /** Indexes every declared node by NodeId, preserving document order. */
    private static Map<String, NodeSetNode> indexNodes(Element root) {
        Map<String, NodeSetNode> rv = new LinkedHashMap<>();
        NodeList children = root.getChildNodes();
        for (int i = 0; i < children.getLength(); ++i) {
            Node node = children.item(i);
            if (node.getNodeType() != Node.ELEMENT_NODE) {
                continue;
            }
            Element element = (Element) node;
            String tag = localName(element.getTagName());
            if (!NODE_ELEMENTS.contains(tag)) {
                continue;
            }
            String nodeId = element.getAttribute("NodeId").trim();
            if (nodeId.isEmpty()) {
                throw new IllegalStateException("NodeSet2 <" + tag + "> element with browse name '"
                    + element.getAttribute("BrowseName") + "' has no NodeId attribute");
            }
            String browseName = qualifiedBrowseName(element.getAttribute("BrowseName"));
            if (browseName == null) {
                if ("UAVariable".equals(tag) || "UAMethod".equals(tag)) {
                    throw new IllegalStateException("NodeSet2 <" + tag + "> node '" + nodeId
                        + "' has no BrowseName attribute");
                }
                // a non-endpoint node only contributes a browse path segment; keep it addressable
                browseName = nodeId;
            }
            if (rv.put(nodeId, new NodeSetNode(element, tag, nodeId, browseName)) != null) {
                throw new IllegalStateException("NodeSet2 node '" + nodeId + "' is declared more than once");
            }
        }
        return rv;
    }

    /** Resolves each node's containing node from its ParentNodeId or its inverse containment reference. */
    private static void resolveHierarchy(Map<String, NodeSetNode> nodes, Map<String, String> aliases) {
        for (NodeSetNode node : nodes.values()) {
            String parentNodeId = node.element.getAttribute("ParentNodeId").trim();
            if (parentNodeId.isEmpty()) {
                parentNodeId = inverseContainmentParent(node.element, aliases);
            }
            node.parentNodeId = parentNodeId.isEmpty() ? null : parentNodeId;
        }
    }

    /** Returns the target of the node's first inverse containment reference, or "" when it has none. */
    private static String inverseContainmentParent(Element element, Map<String, String> aliases) {
        Element references = firstChild(element, "References");
        if (references == null) {
            return "";
        }
        for (Element reference : childElements(references, "Reference")) {
            // a forward reference points at a child; only an inverse one names the containing node
            if (parseBoolean(reference.getAttribute("IsForward"), true)) {
                continue;
            }
            if (!CONTAINMENT_REFERENCES.contains(resolveAlias(reference.getAttribute("ReferenceType"),
                    aliases))) {
                continue;
            }
            String target = reference.getTextContent().trim();
            if (!target.isEmpty()) {
                return target;
            }
        }
        return "";
    }

    /**
     * Folds each method's {@code InputArguments} / {@code OutputArguments} properties into argument
     * lists keyed by "&lt;method NodeId&gt;/&lt;property browse name&gt;", and records the property node
     * ids so that they are not imported as variable endpoints.
     */
    private static void collectMethodArguments(Map<String, NodeSetNode> nodes, Map<String, String> aliases,
            Map<String, List<Object>> methodArguments, Set<String> argumentProperties) {
        for (NodeSetNode node : nodes.values()) {
            if (!"UAVariable".equals(node.tag) || node.parentNodeId == null) {
                continue;
            }
            if (!INPUT_ARGUMENTS.equals(node.browseName) && !OUTPUT_ARGUMENTS.equals(node.browseName)) {
                continue;
            }
            NodeSetNode method = nodes.get(node.parentNodeId);
            if (method == null || !"UAMethod".equals(method.tag)) {
                continue;
            }
            methodArguments.put(method.nodeId + "/" + node.browseName,
                parseArguments(node.element, node.nodeId, aliases));
            argumentProperties.add(node.nodeId);
        }
    }

    /** Parses an {@code Argument[]} property value into the snapshot's argument metadata list. */
    private static List<Object> parseArguments(Element element, String nodeId,
            Map<String, String> aliases) {
        List<Object> rv = new ArrayList<>();
        Element value = firstChild(element, "Value");
        // a NodeSet2 argument property may be declared without a value; it then carries no metadata
        Element list = value != null ? firstChild(value, "ListOfExtensionObject") : null;
        if (list == null) {
            return rv;
        }
        for (Element extension : childElements(list, "ExtensionObject")) {
            Element body = firstChild(extension, "Body");
            Element argument = body != null ? firstChild(body, "Argument") : null;
            if (argument != null) {
                rv.add(parseArgument(argument, nodeId, aliases));
            }
        }
        return rv;
    }

    /** Parses a single OPC UA {@code Argument} structure, matching the live resolver's argument shape. */
    private static Hash parseArgument(Element argument, String nodeId, Map<String, String> aliases) {
        String name = childText(argument, "Name");
        if (name == null) {
            throw new IllegalStateException("NodeSet2 argument in node '" + nodeId + "' has no Name");
        }
        Element dataTypeElement = firstChild(argument, "DataType");
        String dataType = dataTypeElement != null ? childText(dataTypeElement, "Identifier") : null;
        if (dataType == null) {
            throw new IllegalStateException("NodeSet2 argument '" + name + "' in node '" + nodeId
                + "' has no DataType identifier");
        }
        Element description = firstChild(argument, "Description");

        Hash rv = new Hash();
        rv.put("name", name);
        rv.put("description", description != null ? childText(description, "Text") : null);
        rv.put("data_type", DataTypeNames.canonical(resolveAlias(dataType, aliases)));
        String valueRank = childText(argument, "ValueRank");
        rv.put("value_rank", valueRank != null ? Integer.parseInt(valueRank) : -1);
        return rv;
    }

    private static void addEndpoint(List<Object> endpoints, Set<String> seen, List<String> uris,
            Map<String, NodeSetNode> nodes, NodeSetNode node, String kind, Map<String, String> aliases,
            Map<String, List<Object>> methodArguments) throws Exception {
        Element element = node.element;
        String nodeId = node.nodeId;
        int nsIndex = namespaceIndexOf(nodeId);
        if (nsIndex >= uris.size()) {
            // fail with a clear parse error rather than deriving an endpoint id from a null namespace
            throw new IllegalStateException("NodeSet2 node '" + nodeId + "' references namespace index "
                + nsIndex + ", which is not declared in the model's NamespaceUris table");
        }
        String namespaceUri = uris.get(nsIndex);
        String browsePath = browsePathOf(node, nodes);
        String endpointId = SchemaResolver.deriveEndpointId(namespaceUri, browsePath, kind);
        if (!seen.add(endpointId)) {
            throw new IllegalStateException("duplicate imported endpoint id " + endpointId
                + " for namespace " + namespaceUri + " path " + browsePath + " kind " + kind);
        }

        Hash endpoint = new Hash();
        endpoint.put("endpoint_id", endpointId);
        endpoint.put("node_id", nodeId);
        endpoint.put("browse_name", node.browseName);
        endpoint.put("browse_path", browsePath);
        endpoint.put("namespace_uri", namespaceUri);
        endpoint.put("kind", kind);
        endpoint.put("node_class", "variable".equals(kind) ? "Variable" : "Method");
        Element displayName = firstChild(element, "DisplayName");
        endpoint.put("display_name", displayName != null ? displayName.getTextContent().trim() : null);
        if ("variable".equals(kind)) {
            String dataType = element.getAttribute("DataType");
            endpoint.put("data_type", dataType.isEmpty() ? null
                : DataTypeNames.canonical(resolveAlias(dataType, aliases)));
            String valueRank = element.getAttribute("ValueRank");
            endpoint.put("value_rank", valueRank.isEmpty() ? -1 : Integer.parseInt(valueRank));
            String arrayDimensions = element.getAttribute("ArrayDimensions");
            endpoint.put("array_dimensions", parseArrayDimensions(arrayDimensions));
            int accessLevel = parseAccessLevel(element.getAttribute("AccessLevel"), 1);
            int userAccessLevel = parseAccessLevel(element.getAttribute("UserAccessLevel"), accessLevel);
            boolean readable = (accessLevel & 1) != 0;
            boolean writable = (accessLevel & 2) != 0;
            boolean historizing = parseBoolean(element.getAttribute("Historizing"), false);
            endpoint.put("access_level", accessLevel);
            endpoint.put("user_access_level", userAccessLevel);
            endpoint.put("readable", readable);
            endpoint.put("writable", writable);
            endpoint.put("user_writable", (userAccessLevel & 2) != 0);
            endpoint.put("historizing", historizing);
            List<Object> directions = new ArrayList<>();
            if (readable) {
                directions.add("read");
                directions.add("observe");
            }
            if (writable) {
                directions.add("write");
            }
            if (historizing) {
                directions.add("history-read");
            }
            endpoint.put("directions", directions);
        } else {
            endpoint.put("input_arguments",
                methodArguments.getOrDefault(nodeId + "/" + INPUT_ARGUMENTS, new ArrayList<>()));
            endpoint.put("output_arguments",
                methodArguments.getOrDefault(nodeId + "/" + OUTPUT_ARGUMENTS, new ArrayList<>()));
            NodeSetNode parent = node.parentNodeId != null ? nodes.get(node.parentNodeId) : null;
            if (parent != null) {
                endpoint.put("object_node_id", parent.nodeId);
            }
        }
        endpoints.add(endpoint);
    }

    /**
     * Builds a node's namespace-qualified browse path by walking its containment chain to the top of
     * the model, exactly as {@link SchemaResolver} builds one while browsing a live server.
     *
     * <p>A chain that leaves the model keeps a segment for the external parent: its standard browse name
     * when it is a well-known address-space node, and otherwise its NodeId, so that two nodes with the
     * same browse name under different external parents never collapse onto the same path.
     */
    private static String browsePathOf(NodeSetNode node, Map<String, NodeSetNode> nodes) {
        Deque<String> segments = new ArrayDeque<>();
        Set<String> visited = new HashSet<>();
        NodeSetNode current = node;
        while (current != null) {
            if (!visited.add(current.nodeId)) {
                throw new IllegalStateException("NodeSet2 node '" + node.nodeId
                    + "' has a cyclic parent chain through node '" + current.nodeId + "'");
            }
            if (segments.size() >= MAX_PATH_DEPTH) {
                throw new IllegalStateException("NodeSet2 node '" + node.nodeId
                    + "' is nested more than " + MAX_PATH_DEPTH + " levels deep");
            }
            segments.addFirst(current.browseName);
            if (current.parentNodeId == null) {
                break;
            }
            NodeSetNode parent = nodes.get(current.parentNodeId);
            if (parent == null) {
                String segment = STANDARD_PARENTS.getOrDefault(current.parentNodeId, current.parentNodeId);
                if (!segment.isEmpty()) {
                    segments.addFirst(segment);
                }
                break;
            }
            current = parent;
        }
        StringBuilder rv = new StringBuilder();
        for (String segment : segments) {
            rv.append('/').append(segment);
        }
        return rv.toString();
    }

    /**
     * Returns a namespace-qualified browse name (\c N:Name) for a NodeSet2 {@code BrowseName} attribute,
     * or null when the attribute is absent.
     *
     * <p>A NodeSet2 qualified name omits the namespace index when it is 0; making it explicit keeps
     * imported browse names and browse path segments in the same form the live resolver produces.
     */
    private static String qualifiedBrowseName(String browseName) {
        if (browseName == null) {
            return null;
        }
        String trimmed = browseName.trim();
        if (trimmed.isEmpty()) {
            return null;
        }
        return trimmed.indexOf(':') >= 0 ? trimmed : "0:" + trimmed;
    }

    /** Resolves a NodeSet2 alias to its NodeId; a value that is not an alias is returned unchanged. */
    private static String resolveAlias(String value, Map<String, String> aliases) {
        if (value == null) {
            return "";
        }
        String trimmed = value.trim();
        String resolved = aliases.get(trimmed);
        return resolved != null ? resolved : trimmed;
    }

    private static int parseAccessLevel(String value, int defaultValue) {
        if (value == null || value.isEmpty()) {
            return defaultValue;
        }
        int parsed = Integer.parseInt(value);
        if (parsed < 0 || parsed > 255) {
            throw new IllegalArgumentException("NodeSet2 access level must be between 0 and 255; got " + value);
        }
        return parsed;
    }

    private static boolean parseBoolean(String value, boolean defaultValue) {
        if (value == null || value.isEmpty()) {
            return defaultValue;
        }
        if ("true".equalsIgnoreCase(value) || "1".equals(value)) {
            return true;
        }
        if ("false".equalsIgnoreCase(value) || "0".equals(value)) {
            return false;
        }
        throw new IllegalArgumentException("NodeSet2 boolean attributes must be true, false, 1, or 0; got "
            + value);
    }

    private static List<Object> parseArrayDimensions(String value) {
        List<Object> dimensions = new ArrayList<>();
        if (value == null || value.isEmpty()) {
            return dimensions;
        }
        for (String part : value.split(",")) {
            int dimension = Integer.parseInt(part.trim());
            if (dimension < 0) {
                throw new IllegalArgumentException("NodeSet2 array dimensions must be non-negative; got " + value);
            }
            dimensions.add(dimension);
        }
        return dimensions;
    }

    /** Returns the namespace index of a NodeId string (\c ns=N;... ; defaults to 0). */
    private static int namespaceIndexOf(String nodeId) {
        if (nodeId != null && nodeId.startsWith("ns=")) {
            int semi = nodeId.indexOf(';');
            if (semi > 3) {
                try {
                    return Integer.parseInt(nodeId.substring(3, semi));
                } catch (NumberFormatException e) {
                    return 0;
                }
            }
        }
        return 0;
    }

    /** Returns the trimmed text of the first child element with the given local name, or null. */
    private static String childText(Element parent, String tag) {
        Element child = firstChild(parent, tag);
        if (child == null) {
            return null;
        }
        String text = child.getTextContent().trim();
        return text.isEmpty() ? null : text;
    }

    private static Element firstChild(Element parent, String tag) {
        List<Element> elements = childElements(parent, tag);
        return elements.isEmpty() ? null : elements.get(0);
    }

    private static List<Element> childElements(Element parent, String tag) {
        List<Element> rv = new ArrayList<>();
        NodeList children = parent.getChildNodes();
        for (int i = 0; i < children.getLength(); ++i) {
            Node node = children.item(i);
            if (node.getNodeType() == Node.ELEMENT_NODE
                    && localName(((Element) node).getTagName()).equals(tag)) {
                rv.add((Element) node);
            }
        }
        return rv;
    }

    /**
     * Returns the local name of an XML tag (the part after any {@code prefix:}).
     *
     * <p>Namespace awareness is disabled while parsing, so a tag may arrive either bare
     * (e.g. {@code UAVariable}) or prefixed (e.g. {@code u:UAVariable}) depending on how the NodeSet2
     * document is serialized. Matching on the local name makes import robust across both forms.
     *
     * @param tagName the raw tag name as returned by {@link Element#getTagName()}
     * @return the local name with any namespace prefix removed
     */
    private static String localName(String tagName) {
        int colon = tagName.indexOf(':');
        return colon >= 0 ? tagName.substring(colon + 1) : tagName;
    }
}
