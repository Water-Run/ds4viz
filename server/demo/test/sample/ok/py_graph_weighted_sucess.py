# py_graph_weighted_success.py
import ds4viz as dv

dv.config(
    output_path="trace.toml",
    title="Python Weighted Graph Complex Success",
    author="WaterRun",
    comment="Build/update/remove nodes/edges with weights; no runtime errors expected"
)

with dv.graph_weighted(label="demo_weighted_graph", directed=False) as g:
    # Add nodes 0..7
    labels = ["A", "B", "C", "D", "E", "F", "G", "H"]
    for i, lab in enumerate(labels):
        g.add_node(i, lab)

    # Add edges
    g.add_edge(0, 1, 3.5)
    g.add_edge(1, 2, 2.0)
    g.add_edge(2, 3, 4.8)
    g.add_edge(3, 4, 1.2)
    g.add_edge(4, 5, 2.6)
    g.add_edge(5, 6, 3.1)
    g.add_edge(6, 7, 1.9)
    g.add_edge(0, 7, 5.0)
    g.add_edge(0, 2, 2.7)
    g.add_edge(2, 5, 3.3)
    g.add_edge(1, 6, 4.4)

    # Update weights
    g.update_weight(0, 1, 3.0)
    g.update_weight(2, 5, 2.9)
    g.update_weight(0, 7, 4.6)

    # Update node label
    g.update_node_label(3, "D*")

    # Remove and re-add edge
    g.remove_edge(1, 6)
    g.add_edge(1, 6, 4.1)

    # Remove node 4 (cascades to delete edges (3,4) and (4,5))
    g.remove_node(4)

    # Reconnect graph
    g.add_edge(3, 5, 2.2)
    g.add_edge(0, 3, 3.8)
    g.update_weight(5, 6, 2.8)
    g.update_node_label(7, "H*")
    g.remove_edge(0, 2)
