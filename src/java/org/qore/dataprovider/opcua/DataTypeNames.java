/*  DataTypeNames.java Copyright 2026 Qore Technologies, s.r.o.

    Canonical OPC UA data type values for address-space documents.

    An OPC UA data type is a NodeId, but every built-in type also has a well-known name that address
    spaces, NodeSet2 aliases, and write hints are written with.  A document that reports "Double" for a
    hand-authored node and "i=11" for the same type imported from a model describes one type in two
    ways, so a consumer comparing the two sees different types; this class defines the single form the
    document carries and accepts either spelling on the way in.

    Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
    associated documentation files (the "Software"), to deal in the Software without restriction. THE
    SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND.
*/

package org.qore.dataprovider.opcua;

import java.util.Collections;
import java.util.HashMap;
import java.util.Map;

import org.eclipse.milo.opcua.stack.core.OpcUaDataType;
import org.eclipse.milo.opcua.stack.core.types.builtin.NodeId;

/**
 * Resolves and canonicalizes the {@code data_type} of an address-space endpoint or method argument.
 *
 * <p>The canonical form of a built-in OPC UA type is its name ({@code Double}); the canonical form of
 * any other type is its parseable NodeId ({@code i=887}, {@code ns=1;i=31}).  Both spellings of a
 * built-in type are accepted wherever a data type is read, so documents written either way stay valid.
 */
public final class DataTypeNames {
    /**
     * The built-in types whose address-space browse name - the name a NodeSet2 alias and a hand-written
     * document use - differs from the Milo enum constant, which carries the encoding's name instead.
     */
    private static final Map<OpcUaDataType, String> BROWSE_NAMES = Map.of(
        OpcUaDataType.ExtensionObject, "Structure",     // i=22
        OpcUaDataType.Variant, "BaseDataType");         // i=24

    /** Every accepted built-in type name (both spellings) mapped to its standard NodeId. */
    private static final Map<String, NodeId> NAME_TO_NODE_ID;

    /** Each built-in type's NodeId mapped to its canonical name. */
    private static final Map<NodeId, String> NODE_ID_TO_NAME;

    static {
        Map<String, NodeId> names = new HashMap<>();
        Map<NodeId, String> ids = new HashMap<>();
        for (OpcUaDataType type : OpcUaDataType.values()) {
            String browseName = BROWSE_NAMES.getOrDefault(type, type.name());
            names.put(browseName, type.getNodeId());
            names.put(type.name(), type.getNodeId());
            ids.put(type.getNodeId(), browseName);
        }
        NAME_TO_NODE_ID = Collections.unmodifiableMap(names);
        NODE_ID_TO_NAME = Collections.unmodifiableMap(ids);
    }

    private DataTypeNames() {
    }

    /**
     * Resolves a data type written as a built-in type name or as a NodeId.
     *
     * @param dataType the data type as written in the document
     * @return the data type's NodeId
     * @throws IllegalArgumentException if the value is null, empty, or neither a built-in type name nor
     *     a parseable NodeId
     */
    public static NodeId toNodeId(String dataType) {
        if (dataType == null) {
            throw new IllegalArgumentException("a data type is required");
        }
        String trimmed = dataType.trim();
        if (trimmed.isEmpty()) {
            throw new IllegalArgumentException("a data type is required");
        }
        NodeId builtin = NAME_TO_NODE_ID.get(trimmed);
        return builtin != null ? builtin : NodeId.parse(trimmed);
    }

    /**
     * Returns the canonical form of a data type written as a built-in type name or as a NodeId.
     *
     * @param dataType the data type as written in the document, which may be null
     * @return the built-in type name for a built-in type, the parseable NodeId for any other resolvable
     *     type, and the value unchanged when it resolves to neither, so that validation reports it
     */
    public static String canonical(String dataType) {
        if (dataType == null) {
            return null;
        }
        try {
            return canonical(toNodeId(dataType));
        } catch (RuntimeException e) {
            return dataType;
        }
    }

    /**
     * Returns the canonical form of a resolved data type.
     *
     * @param dataType the data type's NodeId, which may be null
     * @return the built-in type name for a built-in type, otherwise the parseable NodeId
     */
    public static String canonical(NodeId dataType) {
        if (dataType == null) {
            return null;
        }
        String name = NODE_ID_TO_NAME.get(dataType);
        return name != null ? name : dataType.toParseableString();
    }

    /**
     * Returns the built-in scalar type name the value codec builds values for.
     *
     * @param dataType the data type's NodeId, which may be null
     * @return the scalar type name, or null when the type is not one the codec builds
     */
    public static String scalarName(NodeId dataType) {
        String name = dataType != null ? NODE_ID_TO_NAME.get(dataType) : null;
        return name != null && ValueCodec.isSupportedScalarType(name) ? name : null;
    }
}
