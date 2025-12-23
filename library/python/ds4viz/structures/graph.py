r"""
图数据结构实现：无向图、有向图、带权图

:file: ds4viz/structures/graph.py
:author: WaterRun
:time: 2025-12-23
"""

from typing import Any

from ds4viz.structures._base import BaseStructure
from ds4viz.session import get_caller_line


class GraphUndirected(BaseStructure):
    r"""
    无向图数据结构
    """

    def __init__(self, label: str = "graph", output: str | None = None) -> None:
        r"""
        初始化无向图

        :param label: 无向图的标签
        :param output: 输出文件路径
        :return None: 无返回值
        """
        super().__init__("graph_undirected", label, output)
        self._nodes: dict[int, str] = {}
        self._edges: set[tuple[int, int]] = set()

    def _build_data(self) -> dict[str, Any]:
        r"""
        构建无向图的状态数据

        :return dict[str, Any]: 状态数据字典
        """
        nodes_list: list[dict[str, Any]] = [
            {"id": nid, "label": label}
            for nid, label in sorted(self._nodes.items())
        ]
        edges_list: list[dict[str, int]] = [
            {"from": f, "to": t}
            for f, t in sorted(self._edges)
        ]
        return {"nodes": nodes_list, "edges": edges_list}

    @staticmethod
    def _normalize_edge(from_id: int, to_id: int) -> tuple[int, int]:
        r"""
        规范化无向边，确保 from < to

        :param from_id: 起始节点 id
        :param to_id: 终止节点 id
        :return tuple[int, int]: 规范化后的边
        """
        return (from_id, to_id) if from_id < to_id else (to_id, from_id)

    def add_node(self, node_id: int, label: str) -> None:
        r"""
        添加节点

        :param node_id: 节点 id
        :param label: 节点标签，长度限制为 1-32 字符
        :return None: 无返回值
        :raise RuntimeError: 当节点已存在时抛出异常
        """
        if node_id in self._nodes:
            self._raise_error(f"Node already exists: {node_id}")
        if not (1 <= len(label) <= 32):
            self._raise_error(f"Label length must be 1-32, got {len(label)}")
        before: int = self._session.get_last_state_id()
        line: int = get_caller_line(2)
        self._nodes[node_id] = label
        after: int = self._session.add_state(self._build_data())
        self._session.add_step(
            op="add_node",
            before=before,
            after=after,
            args={"id": node_id, "label": label},
            line=line
        )

    def add_edge(self, from_id: int, to_id: int) -> None:
        r"""
        添加无向边，内部自动规范化为 from_id < to_id

        :param from_id: 起始节点 id
        :param to_id: 终止节点 id
        :return None: 无返回值
        :raise RuntimeError: 当节点不存在、边已存在或形成自环时抛出异常
        """
        if from_id not in self._nodes:
            self._raise_error(f"Node not found: {from_id}")
        if to_id not in self._nodes:
            self._raise_error(f"Node not found: {to_id}")
        if from_id == to_id:
            self._raise_error(f"Self-loop not allowed: {from_id}")
        edge: tuple[int, int] = self._normalize_edge(from_id, to_id)
        if edge in self._edges:
            self._raise_error(f"Edge already exists: {edge}")
        before: int = self._session.get_last_state_id()
        line: int = get_caller_line(2)
        self._edges.add(edge)
        after: int = self._session.add_state(self._build_data())
        self._session.add_step(
            op="add_edge",
            before=before,
            after=after,
            args={"from": from_id, "to": to_id},
            line=line
        )

    def remove_node(self, node_id: int) -> None:
        r"""
        删除节点及其相关的所有边

        :param node_id: 要删除的节点 id
        :return None: 无返回值
        :raise RuntimeError: 当节点不存在时抛出异常
        """
        if node_id not in self._nodes:
            self._raise_error(f"Node not found: {node_id}")
        before: int = self._session.get_last_state_id()
        line: int = get_caller_line(2)
        edges_to_remove: set[tuple[int, int]] = {
            e for e in self._edges if node_id in e
        }
        self._edges -= edges_to_remove
        del self._nodes[node_id]
        after: int = self._session.add_state(self._build_data())
        self._session.add_step(
            op="remove_node",
            before=before,
            after=after,
            args={"node_id": node_id},
            line=line
        )

    def remove_edge(self, from_id: int, to_id: int) -> None:
        r"""
        删除边

        :param from_id: 起始节点 id
        :param to_id: 终止节点 id
        :return None: 无返回值
        :raise RuntimeError: 当边不存在时抛出异常
        """
        edge: tuple[int, int] = self._normalize_edge(from_id, to_id)
        if edge not in self._edges:
            self._raise_error(f"Edge not found: {edge}")
        before: int = self._session.get_last_state_id()
        line: int = get_caller_line(2)
        self._edges.remove(edge)
        after: int = self._session.add_state(self._build_data())
        self._session.add_step(
            op="remove_edge",
            before=before,
            after=after,
            args={"from": from_id, "to": to_id},
            line=line
        )

    def update_node_label(self, node_id: int, label: str) -> None:
        r"""
        更新节点标签

        :param node_id: 节点 id
        :param label: 新的节点标签，长度限制为 1-32 字符
        :return None: 无返回值
        :raise RuntimeError: 当节点不存在时抛出异常
        """
        if node_id not in self._nodes:
            self._raise_error(f"Node not found: {node_id}")
        if not (1 <= len(label) <= 32):
            self._raise_error(f"Label length must be 1-32, got {len(label)}")
        before: int = self._session.get_last_state_id()
        line: int = get_caller_line(2)
        old_label: str = self._nodes[node_id]
        self._nodes[node_id] = label
        after: int = self._session.add_state(self._build_data())
        self._session.add_step(
            op="update_node_label",
            before=before,
            after=after,
            args={"node_id": node_id, "label": label},
            ret=old_label,
            line=line
        )


class GraphDirected(BaseStructure):
    r"""
    有向图数据结构
    """

    def __init__(self, label: str = "graph", output: str | None = None) -> None:
        r"""
        初始化有向图

        :param label: 有向图的标签
        :param output: 输出文件路径
        :return None: 无返回值
        """
        super().__init__("graph_directed", label, output)
        self._nodes: dict[int, str] = {}
        self._edges: set[tuple[int, int]] = set()

    def _build_data(self) -> dict[str, Any]:
        r"""
        构建有向图的状态数据

        :return dict[str, Any]: 状态数据字典
        """
        nodes_list: list[dict[str, Any]] = [
            {"id": nid, "label": label}
            for nid, label in sorted(self._nodes.items())
        ]
        edges_list: list[dict[str, int]] = [
            {"from": f, "to": t}
            for f, t in sorted(self._edges)
        ]
        return {"nodes": nodes_list, "edges": edges_list}

    def add_node(self, node_id: int, label: str) -> None:
        r"""
        添加节点

        :param node_id: 节点 id
        :param label: 节点标签，长度限制为 1-32 字符
        :return None: 无返回值
        :raise RuntimeError: 当节点已存在时抛出异常
        """
        if node_id in self._nodes:
            self._raise_error(f"Node already exists: {node_id}")
        if not (1 <= len(label) <= 32):
            self._raise_error(f"Label length must be 1-32, got {len(label)}")
        before: int = self._session.get_last_state_id()
        line: int = get_caller_line(2)
        self._nodes[node_id] = label
        after: int = self._session.add_state(self._build_data())
        self._session.add_step(
            op="add_node",
            before=before,
            after=after,
            args={"id": node_id, "label": label},
            line=line
        )

    def add_edge(self, from_id: int, to_id: int) -> None:
        r"""
        添加有向边

        :param from_id: 起始节点 id
        :param to_id: 终止节点 id
        :return None: 无返回值
        :raise RuntimeError: 当节点不存在、边已存在或形成自环时抛出异常
        """
        if from_id not in self._nodes:
            self._raise_error(f"Node not found: {from_id}")
        if to_id not in self._nodes:
            self._raise_error(f"Node not found: {to_id}")
        if from_id == to_id:
            self._raise_error(f"Self-loop not allowed: {from_id}")
        edge: tuple[int, int] = (from_id, to_id)
        if edge in self._edges:
            self._raise_error(f"Edge already exists: {edge}")
        before: int = self._session.get_last_state_id()
        line: int = get_caller_line(2)
        self._edges.add(edge)
        after: int = self._session.add_state(self._build_data())
        self._session.add_step(
            op="add_edge",
            before=before,
            after=after,
            args={"from": from_id, "to": to_id},
            line=line
        )

    def remove_node(self, node_id: int) -> None:
        r"""
        删除节点及其相关的所有边

        :param node_id: 要删除的节点 id
        :return None: 无返回值
        :raise RuntimeError: 当节点不存在时抛出异常
        """
        if node_id not in self._nodes:
            self._raise_error(f"Node not found: {node_id}")
        before: int = self._session.get_last_state_id()
        line: int = get_caller_line(2)
        edges_to_remove: set[tuple[int, int]] = {
            e for e in self._edges if node_id in e
        }
        self._edges -= edges_to_remove
        del self._nodes[node_id]
        after: int = self._session.add_state(self._build_data())
        self._session.add_step(
            op="remove_node",
            before=before,
            after=after,
            args={"node_id": node_id},
            line=line
        )

    def remove_edge(self, from_id: int, to_id: int) -> None:
        r"""
        删除边

        :param from_id: 起始节点 id
        :param to_id: 终止节点 id
        :return None: 无返回值
        :raise RuntimeError: 当边不存在时抛出异常
        """
        edge: tuple[int, int] = (from_id, to_id)
        if edge not in self._edges:
            self._raise_error(f"Edge not found: {edge}")
        before: int = self._session.get_last_state_id()
        line: int = get_caller_line(2)
        self._edges.remove(edge)
        after: int = self._session.add_state(self._build_data())
        self._session.add_step(
            op="remove_edge",
            before=before,
            after=after,
            args={"from": from_id, "to": to_id},
            line=line
        )

    def update_node_label(self, node_id: int, label: str) -> None:
        r"""
        更新节点标签

        :param node_id: 节点 id
        :param label: 新的节点标签，长度限制为 1-32 字符
        :return None: 无返回值
        :raise RuntimeError: 当节点不存在时抛出异常
        """
        if node_id not in self._nodes:
            self._raise_error(f"Node not found: {node_id}")
        if not (1 <= len(label) <= 32):
            self._raise_error(f"Label length must be 1-32, got {len(label)}")
        before: int = self._session.get_last_state_id()
        line: int = get_caller_line(2)
        old_label: str = self._nodes[node_id]
        self._nodes[node_id] = label
        after: int = self._session.add_state(self._build_data())
        self._session.add_step(
            op="update_node_label",
            before=before,
            after=after,
            args={"node_id": node_id, "label": label},
            ret=old_label,
            line=line
        )


class GraphWeighted(BaseStructure):
    r"""
    带权图数据结构
    """

    def __init__(
        self,
        label: str = "graph",
        directed: bool = False,
        output: str | None = None
    ) -> None:
        r"""
        初始化带权图

        :param label: 带权图的标签
        :param directed: 是否为有向图，默认为 False（无向图）
        :param output: 输出文件路径
        :return None: 无返回值
        """
        super().__init__("graph_weighted", label, output)
        self._directed: bool = directed
        self._nodes: dict[int, str] = {}
        self._edges: dict[tuple[int, int], float] = {}

    def _build_data(self) -> dict[str, Any]:
        r"""
        构建带权图的状态数据

        :return dict[str, Any]: 状态数据字典
        """
        nodes_list: list[dict[str, Any]] = [
            {"id": nid, "label": label}
            for nid, label in sorted(self._nodes.items())
        ]
        edges_list: list[dict[str, Any]] = [
            {"from": f, "to": t, "weight": w}
            for (f, t), w in sorted(self._edges.items())
        ]
        return {"nodes": nodes_list, "edges": edges_list}

    def _normalize_edge(self, from_id: int, to_id: int) -> tuple[int, int]:
        r"""
        规范化边（无向图时确保 from < to）

        :param from_id: 起始节点 id
        :param to_id: 终止节点 id
        :return tuple[int, int]: 规范化后的边
        """
        if self._directed:
            return from_id, to_id
        return (from_id, to_id) if from_id < to_id else (to_id, from_id)

    def add_node(self, node_id: int, label: str) -> None:
        r"""
        添加节点

        :param node_id: 节点 id
        :param label: 节点标签，长度限制为 1-32 字符
        :return None: 无返回值
        :raise RuntimeError: 当节点已存在时抛出异常
        """
        if node_id in self._nodes:
            self._raise_error(f"Node already exists: {node_id}")
        if not (1 <= len(label) <= 32):
            self._raise_error(f"Label length must be 1-32, got {len(label)}")
        before: int = self._session.get_last_state_id()
        line: int = get_caller_line(2)
        self._nodes[node_id] = label
        after: int = self._session.add_state(self._build_data())
        self._session.add_step(
            op="add_node",
            before=before,
            after=after,
            args={"id": node_id, "label": label},
            line=line
        )

    def add_edge(self, from_id: int, to_id: int, weight: float) -> None:
        r"""
        添加带权边

        :param from_id: 起始节点 id
        :param to_id: 终止节点 id
        :param weight: 边的权重
        :return None: 无返回值
        :raise RuntimeError: 当节点不存在、边已存在或形成自环时抛出异常
        """
        if from_id not in self._nodes:
            self._raise_error(f"Node not found: {from_id}")
        if to_id not in self._nodes:
            self._raise_error(f"Node not found: {to_id}")
        if from_id == to_id:
            self._raise_error(f"Self-loop not allowed: {from_id}")
        edge: tuple[int, int] = self._normalize_edge(from_id, to_id)
        if edge in self._edges:
            self._raise_error(f"Edge already exists: {edge}")
        before: int = self._session.get_last_state_id()
        line: int = get_caller_line(2)
        self._edges[edge] = weight
        after: int = self._session.add_state(self._build_data())
        self._session.add_step(
            op="add_edge",
            before=before,
            after=after,
            args={"from": from_id, "to": to_id, "weight": weight},
            line=line
        )

    def remove_node(self, node_id: int) -> None:
        r"""
        删除节点及其相关的所有边

        :param node_id: 要删除的节点 id
        :return None: 无返回值
        :raise RuntimeError: 当节点不存在时抛出异常
        """
        if node_id not in self._nodes:
            self._raise_error(f"Node not found: {node_id}")
        before: int = self._session.get_last_state_id()
        line: int = get_caller_line(2)
        edges_to_remove: list[tuple[int, int]] = [
            e for e in self._edges if node_id in e
        ]
        for e in edges_to_remove:
            del self._edges[e]
        del self._nodes[node_id]
        after: int = self._session.add_state(self._build_data())
        self._session.add_step(
            op="remove_node",
            before=before,
            after=after,
            args={"node_id": node_id},
            line=line
        )

    def remove_edge(self, from_id: int, to_id: int) -> None:
        r"""
        删除边

        :param from_id: 起始节点 id
        :param to_id: 终止节点 id
        :return None: 无返回值
        :raise RuntimeError: 当边不存在时抛出异常
        """
        edge: tuple[int, int] = self._normalize_edge(from_id, to_id)
        if edge not in self._edges:
            self._raise_error(f"Edge not found: {edge}")
        before: int = self._session.get_last_state_id()
        line: int = get_caller_line(2)
        del self._edges[edge]
        after: int = self._session.add_state(self._build_data())
        self._session.add_step(
            op="remove_edge",
            before=before,
            after=after,
            args={"from": from_id, "to": to_id},
            line=line
        )

    def update_weight(self, from_id: int, to_id: int, weight: float) -> None:
        r"""
        更新边的权重

        :param from_id: 起始节点 id
        :param to_id: 终止节点 id
        :param weight: 新的权重值
        :return None: 无返回值
        :raise RuntimeError: 当边不存在时抛出异常
        """
        edge: tuple[int, int] = self._normalize_edge(from_id, to_id)
        if edge not in self._edges:
            self._raise_error(f"Edge not found: {edge}")
        before: int = self._session.get_last_state_id()
        line: int = get_caller_line(2)
        old_weight: float = self._edges[edge]
        self._edges[edge] = weight
        after: int = self._session.add_state(self._build_data())
        self._session.add_step(
            op="update_weight",
            before=before,
            after=after,
            args={"from": from_id, "to": to_id, "weight": weight},
            ret=old_weight,
            line=line
        )

    def update_node_label(self, node_id: int, label: str) -> None:
        r"""
        更新节点标签

        :param node_id: 节点 id
        :param label: 新的节点标签，长度限制为 1-32 字符
        :return None: 无返回值
        :raise RuntimeError: 当节点不存在时抛出异常
        """
        if node_id not in self._nodes:
            self._raise_error(f"Node not found: {node_id}")
        if not (1 <= len(label) <= 32):
            self._raise_error(f"Label length must be 1-32, got {len(label)}")
        before: int = self._session.get_last_state_id()
        line: int = get_caller_line(2)
        old_label: str = self._nodes[node_id]
        self._nodes[node_id] = label
        after: int = self._session.add_state(self._build_data())
        self._session.add_step(
            op="update_node_label",
            before=before,
            after=after,
            args={"node_id": node_id, "label": label},
            ret=old_label,
            line=line
        )
