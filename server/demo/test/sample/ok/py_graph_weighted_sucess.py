# py_graph_weighted_sucess.py
import os
import uuid
from pathlib import Path

import ds4viz as dv


def _make_output_path() -> Path:
    base = Path(os.getenv("DS4VIZ_TEMP_DIR", "/tmp/ds4viz"))
    base.mkdir(parents=True, exist_ok=True)
    return base / f"trace_py_weighted_graph_{uuid.uuid4().hex}.toml"


def _print_and_cleanup(path: Path) -> None:
    try:
        print(path.read_text(encoding="utf-8"), end="")
    finally:
        try:
            path.unlink(missing_ok=True)
        except Exception:
            pass


def main() -> None:
    out = _make_output_path()

    dv.config(
        output_path=str(out),
        title="ds4viz demo: weighted graph (complex)",
        author="WaterRun",
        comment="Build/update/remove nodes/edges with weights; no runtime errors expected",
    )

    # 复杂：节点较多 + 边增删改 + 标签更新 + 删除节点触发边级联删除
    with dv.graph_weighted(label="demo_weighted_graph", directed=False) as g:
        # nodes: 0..7
        labels = ["A", "B", "C", "D", "E", "F", "G", "H"]
        for i, lab in enumerate(labels):
            g.add_node(i, lab)

        # edges (undirected)
        g.add_edge(0, 1, 3.5)
        g.add_edge(1, 2, 2.0)
        g.add_edge(2, 3, 4.8)
        g.add_edge(3, 4, 1.2)
        g.add_edge(4, 5, 2.6)
        g.add_edge(5, 6, 3.1)
        g.add_edge(6, 7, 1.9)
        g.add_edge(0, 7, 5.0)

        # add some cross edges
        g.add_edge(0, 2, 2.7)
        g.add_edge(2, 5, 3.3)
        g.add_edge(1, 6, 4.4)

        # update weights (simulate algorithm relaxation / adjustments)
        g.update_weight(0, 1, 3.0)
        g.update_weight(2, 5, 2.9)
        g.update_weight(0, 7, 4.6)

        # update label
        g.update_node_label(3, "D*")

        # remove an edge then re-add with a different weight
        g.remove_edge(1, 6)
        g.add_edge(1, 6, 4.1)

        # remove node (and related edges)
        g.remove_node(4)

        # after removing node 4, connect around it to keep graph connected
        g.add_edge(3, 5, 2.2)

    _print_and_cleanup(out)


if __name__ == "__main__":
    main()
