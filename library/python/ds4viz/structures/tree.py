r"""
树形数据结构实现：二叉树、二叉搜索树、堆

:file: ds4viz/structures/tree.py
:author: WaterRun
:time: 2025-12-23
"""

from typing import Any

from ds4viz.structures._base import BaseStructure, ValueType
from ds4viz.session import get_caller_line


class BinaryTree(BaseStructure):
    r"""
    二叉树数据结构
    """

    def __init__(self, label: str = "binary_tree", output: str | None = None) -> None:
        r"""
        初始化二叉树

        :param label: 二叉树的标签
        :param output: 输出文件路径
        :return None: 无返回值
        """
        super().__init__("binary_tree", label, output)
        self._nodes: dict[int, dict[str, Any]] = {}
        self._root: int = -1
        self._next_id: int = 0

    def _build_data(self) -> dict[str, Any]:
        r"""
        构建二叉树的状态数据

        :return dict[str, Any]: 状态数据字典
        """
        nodes_list: list[dict[str, Any]] = [
            {"id": nid, "value": data["value"], "left": data["left"], "right": data["right"]}
            for nid, data in sorted(self._nodes.items())
        ]
        return {"root": self._root, "nodes": nodes_list}

    def _find_parent(self, node_id: int) -> int:
        r"""
        查找指定节点的父节点 ID

        :param node_id: 目标节点 ID
        :return int: 父节点 ID，若为根节点或不存在则返回 -1
        """
        for nid, data in self._nodes.items():
            if data["left"] == node_id or data["right"] == node_id:
                return nid
        return -1

    def _collect_subtree(self, node_id: int) -> list[int]:
        r"""
        收集以指定节点为根的子树中所有节点 ID

        :param node_id: 根节点 ID
        :return list[int]: 子树中所有节点 ID 列表
        """
        if node_id == -1:
            return []
        result: list[int] = [node_id]
        result.extend(self._collect_subtree(self._nodes[node_id]["left"]))
        result.extend(self._collect_subtree(self._nodes[node_id]["right"]))
        return result

    def insert_root(self, value: ValueType) -> int:
        r"""
        插入根节点

        :param value: 根节点的值
        :return int: 根节点的 id
        :raise RuntimeError: 当根节点已存在时抛出异常
        """
        if self._root != -1:
            self._raise_error("Root node already exists")
        before: int = self._session.get_last_state_id()
        line: int = get_caller_line(2)
        new_id: int = self._next_id
        self._next_id += 1
        self._nodes[new_id] = {"value": value, "left": -1, "right": -1}
        self._root = new_id
        after: int = self._session.add_state(self._build_data())
        self._session.add_step(
            op="insert_root",
            before=before,
            after=after,
            args={"value": value},
            ret=new_id,
            line=line
        )
        return new_id

    def insert_left(self, parent_id: int, value: ValueType) -> int:
        r"""
        在指定父节点的左侧插入子节点

        :param parent_id: 父节点的 id
        :param value: 要插入的子节点值
        :return int: 新插入节点的 id
        :raise RuntimeError: 当父节点不存在或左子节点已存在时抛出异常
        """
        if parent_id not in self._nodes:
            self._raise_error(f"Parent node not found: {parent_id}")
        if self._nodes[parent_id]["left"] != -1:
            self._raise_error(f"Left child already exists for node: {parent_id}")
        before: int = self._session.get_last_state_id()
        line: int = get_caller_line(2)
        new_id: int = self._next_id
        self._next_id += 1
        self._nodes[new_id] = {"value": value, "left": -1, "right": -1}
        self._nodes[parent_id]["left"] = new_id
        after: int = self._session.add_state(self._build_data())
        self._session.add_step(
            op="insert_left",
            before=before,
            after=after,
            args={"parent": parent_id, "value": value},
            ret=new_id,
            line=line
        )
        return new_id

    def insert_right(self, parent_id: int, value: ValueType) -> int:
        r"""
        在指定父节点的右侧插入子节点

        :param parent_id: 父节点的 id
        :param value: 要插入的子节点值
        :return int: 新插入节点的 id
        :raise RuntimeError: 当父节点不存在或右子节点已存在时抛出异常
        """
        if parent_id not in self._nodes:
            self._raise_error(f"Parent node not found: {parent_id}")
        if self._nodes[parent_id]["right"] != -1:
            self._raise_error(f"Right child already exists for node: {parent_id}")
        before: int = self._session.get_last_state_id()
        line: int = get_caller_line(2)
        new_id: int = self._next_id
        self._next_id += 1
        self._nodes[new_id] = {"value": value, "left": -1, "right": -1}
        self._nodes[parent_id]["right"] = new_id
        after: int = self._session.add_state(self._build_data())
        self._session.add_step(
            op="insert_right",
            before=before,
            after=after,
            args={"parent": parent_id, "value": value},
            ret=new_id,
            line=line
        )
        return new_id

    def delete(self, node_id: int) -> None:
        r"""
        删除指定节点及其所有子树

        :param node_id: 要删除的节点 id
        :return None: 无返回值
        :raise RuntimeError: 当节点不存在时抛出异常
        """
        if node_id not in self._nodes:
            self._raise_error(f"Node not found: {node_id}")
        before: int = self._session.get_last_state_id()
        line: int = get_caller_line(2)
        subtree_ids: list[int] = self._collect_subtree(node_id)
        parent_id: int = self._find_parent(node_id)
        if parent_id != -1:
            if self._nodes[parent_id]["left"] == node_id:
                self._nodes[parent_id]["left"] = -1
            else:
                self._nodes[parent_id]["right"] = -1
        else:
            self._root = -1
        for nid in subtree_ids:
            del self._nodes[nid]
        after: int = self._session.add_state(self._build_data())
        self._session.add_step(
            op="delete",
            before=before,
            after=after,
            args={"node_id": node_id},
            line=line
        )

    def update_value(self, node_id: int, value: ValueType) -> None:
        r"""
        更新节点的值

        :param node_id: 节点 id
        :param value: 新的值
        :return None: 无返回值
        :raise RuntimeError: 当节点不存在时抛出异常
        """
        if node_id not in self._nodes:
            self._raise_error(f"Node not found: {node_id}")
        before: int = self._session.get_last_state_id()
        line: int = get_caller_line(2)
        old_value: ValueType = self._nodes[node_id]["value"]
        self._nodes[node_id]["value"] = value
        after: int = self._session.add_state(self._build_data())
        self._session.add_step(
            op="update_value",
            before=before,
            after=after,
            args={"node_id": node_id, "value": value},
            ret=old_value,
            line=line
        )


class BinarySearchTree(BaseStructure):
    r"""
    二叉搜索树数据结构
    """

    def __init__(self, label: str = "bst", output: str | None = None) -> None:
        r"""
        初始化二叉搜索树

        :param label: 二叉搜索树的标签
        :param output: 输出文件路径
        :return None: 无返回值
        """
        super().__init__("bst", label, output)
        self._nodes: dict[int, dict[str, Any]] = {}
        self._root: int = -1
        self._next_id: int = 0

    def _build_data(self) -> dict[str, Any]:
        r"""
        构建二叉搜索树的状态数据

        :return dict[str, Any]: 状态数据字典
        """
        nodes_list: list[dict[str, Any]] = [
            {"id": nid, "value": data["value"], "left": data["left"], "right": data["right"]}
            for nid, data in sorted(self._nodes.items())
        ]
        return {"root": self._root, "nodes": nodes_list}

    def _find_node_by_value(self, value: int | float) -> int:
        r"""
        根据值查找节点 ID

        :param value: 要查找的值
        :return int: 节点 ID，若不存在则返回 -1
        """
        for nid, data in self._nodes.items():
            if data["value"] == value:
                return nid
        return -1

    def _find_parent(self, node_id: int) -> int:
        r"""
        查找指定节点的父节点 ID

        :param node_id: 目标节点 ID
        :return int: 父节点 ID，若为根节点或不存在则返回 -1
        """
        for nid, data in self._nodes.items():
            if data["left"] == node_id or data["right"] == node_id:
                return nid
        return -1

    def _find_min_node(self, node_id: int) -> int:
        r"""
        查找以指定节点为根的子树中的最小值节点

        :param node_id: 子树根节点 ID
        :return int: 最小值节点 ID
        """
        current: int = node_id
        while self._nodes[current]["left"] != -1:
            current = self._nodes[current]["left"]
        return current

    def insert(self, value: int | float) -> int:
        r"""
        插入节点，自动维护二叉搜索树性质

        :param value: 要插入的值
        :return int: 新插入节点的 id
        """
        before: int = self._session.get_last_state_id()
        line: int = get_caller_line(2)
        new_id: int = self._next_id
        self._next_id += 1
        self._nodes[new_id] = {"value": value, "left": -1, "right": -1}
        if self._root == -1:
            self._root = new_id
        else:
            current: int = self._root
            while True:
                current_value: int | float = self._nodes[current]["value"]
                if value < current_value:
                    if self._nodes[current]["left"] == -1:
                        self._nodes[current]["left"] = new_id
                        break
                    current = self._nodes[current]["left"]
                else:
                    if self._nodes[current]["right"] == -1:
                        self._nodes[current]["right"] = new_id
                        break
                    current = self._nodes[current]["right"]
        after: int = self._session.add_state(self._build_data())
        self._session.add_step(
            op="insert",
            before=before,
            after=after,
            args={"value": value},
            ret=new_id,
            line=line
        )
        return new_id

    def delete(self, value: int | float) -> None:
        r"""
        删除节点，自动维护二叉搜索树性质

        :param value: 要删除的节点值
        :return None: 无返回值
        :raise RuntimeError: 当节点不存在时抛出异常
        """
        node_id: int = self._find_node_by_value(value)
        if node_id == -1:
            self._raise_error(f"Node with value {value} not found")
        before: int = self._session.get_last_state_id()
        line: int = get_caller_line(2)
        self._delete_node(node_id)
        after: int = self._session.add_state(self._build_data())
        self._session.add_step(
            op="delete",
            before=before,
            after=after,
            args={"value": value},
            line=line
        )

    def _delete_node(self, node_id: int) -> None:
        r"""
        删除指定节点并维护 BST 性质

        :param node_id: 要删除的节点 ID
        :return None: 无返回值
        """
        node: dict[str, Any] = self._nodes[node_id]
        parent_id: int = self._find_parent(node_id)
        left_child: int = node["left"]
        right_child: int = node["right"]
        if left_child == -1 and right_child == -1:
            self._replace_in_parent(parent_id, node_id, -1)
            del self._nodes[node_id]
        elif left_child == -1:
            self._replace_in_parent(parent_id, node_id, right_child)
            del self._nodes[node_id]
        elif right_child == -1:
            self._replace_in_parent(parent_id, node_id, left_child)
            del self._nodes[node_id]
        else:
            successor_id: int = self._find_min_node(right_child)
            successor_value: int | float = self._nodes[successor_id]["value"]
            self._delete_node(successor_id)
            self._nodes[node_id]["value"] = successor_value

    def _replace_in_parent(self, parent_id: int, old_child: int, new_child: int) -> None:
        r"""
        在父节点中替换子节点引用

        :param parent_id: 父节点 ID
        :param old_child: 旧子节点 ID
        :param new_child: 新子节点 ID
        :return None: 无返回值
        """
        if parent_id == -1:
            self._root = new_child
        elif self._nodes[parent_id]["left"] == old_child:
            self._nodes[parent_id]["left"] = new_child
        else:
            self._nodes[parent_id]["right"] = new_child


class Heap(BaseStructure):
    r"""
    堆数据结构（使用完全二叉树表示）
    """

    def __init__(
        self,
        label: str = "heap",
        heap_type: str = "min",
        output: str | None = None
    ) -> None:
        r"""
        初始化堆

        :param label: 堆的标签
        :param heap_type: 堆类型，"min" 表示最小堆，"max" 表示最大堆
        :param output: 输出文件路径
        :return None: 无返回值
        :raise ValueError: 当 heap_type 不是 "min" 或 "max" 时抛出异常
        """
        if heap_type not in ("min", "max"):
            raise ValueError(f"heap_type must be 'min' or 'max', got '{heap_type}'")
        super().__init__("binary_tree", label, output)
        self._heap_type: str = heap_type
        self._items: list[int | float] = []

    def _build_data(self) -> dict[str, Any]:
        r"""
        构建堆的状态数据（以完全二叉树形式表示）

        :return dict[str, Any]: 状态数据字典
        """
        if not self._items:
            return {"root": -1, "nodes": []}
        nodes_list: list[dict[str, Any]] = []
        for i, value in enumerate(self._items):
            left: int = 2 * i + 1 if 2 * i + 1 < len(self._items) else -1
            right: int = 2 * i + 2 if 2 * i + 2 < len(self._items) else -1
            nodes_list.append({"id": i, "value": value, "left": left, "right": right})
        return {"root": 0, "nodes": nodes_list}

    def _compare(self, a: int | float, b: int | float) -> bool:
        r"""
        比较两个值，根据堆类型决定比较方式

        :param a: 第一个值
        :param b: 第二个值
        :return bool: 如果 a 应该在 b 之前（更接近堆顶）则返回 True
        """
        return a < b if self._heap_type == "min" else a > b

    def _sift_up(self, index: int) -> None:
        r"""
        向上调整堆

        :param index: 开始调整的索引
        :return None: 无返回值
        """
        while index > 0:
            parent: int = (index - 1) // 2
            if self._compare(self._items[index], self._items[parent]):
                self._items[index], self._items[parent] = self._items[parent], self._items[index]
                index = parent
            else:
                break

    def _sift_down(self, index: int) -> None:
        r"""
        向下调整堆

        :param index: 开始调整的索引
        :return None: 无返回值
        """
        size: int = len(self._items)
        while True:
            target: int = index
            left: int = 2 * index + 1
            right: int = 2 * index + 2
            if left < size and self._compare(self._items[left], self._items[target]):
                target = left
            if right < size and self._compare(self._items[right], self._items[target]):
                target = right
            if target == index:
                break
            self._items[index], self._items[target] = self._items[target], self._items[index]
            index = target

    def insert(self, value: int | float) -> None:
        r"""
        插入元素，自动维护堆性质

        :param value: 要插入的值
        :return None: 无返回值
        """
        before: int = self._session.get_last_state_id()
        line: int = get_caller_line(2)
        self._items.append(value)
        self._sift_up(len(self._items) - 1)
        after: int = self._session.add_state(self._build_data())
        self._session.add_step(
            op="insert",
            before=before,
            after=after,
            args={"value": value},
            line=line
        )

    def extract(self) -> None:
        r"""
        提取堆顶元素，自动维护堆性质

        :return None: 无返回值
        :raise RuntimeError: 当堆为空时抛出异常
        """
        if not self._items:
            self._raise_error("Cannot extract from empty heap")
        before: int = self._session.get_last_state_id()
        line: int = get_caller_line(2)
        extracted: int | float = self._items[0]
        if len(self._items) == 1:
            self._items.pop()
        else:
            self._items[0] = self._items.pop()
            self._sift_down(0)
        after: int = self._session.add_state(self._build_data())
        self._session.add_step(
            op="extract",
            before=before,
            after=after,
            args={},
            ret=extracted,
            line=line
        )

    def clear(self) -> None:
        r"""
        清空堆

        :return None: 无返回值
        """
        before: int = self._session.get_last_state_id()
        line: int = get_caller_line(2)
        self._items.clear()
        after: int = self._session.add_state(self._build_data())
        self._session.add_step(
            op="clear",
            before=before,
            after=after,
            args={},
            line=line
        )
