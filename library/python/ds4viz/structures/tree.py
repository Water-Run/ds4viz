r"""
树形数据结构实现: 二叉树、二叉搜索树

:file: ds4viz/structures/tree.py
:author: WaterRun
:time: 2026-03-23
"""

from typing import Any

from ds4viz.structures._base import BaseStructure, ValueType


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

    def _node_exists(self, node_id: int) -> bool:
        r"""
        检查节点是否存在

        :param node_id: 节点 ID
        :return bool: 节点是否存在
        """
        return node_id in self._nodes

    def _build_data(self) -> dict[str, Any]:
        r"""
        构建二叉树的状态数据

        :return dict[str, Any]: 状态数据字典
        """
        nodes_list: list[dict[str, Any]] = []
        for nid, data in sorted(self._nodes.items()):
            node_dict: dict[str, Any] = {"id": nid}
            if nid in self._aliases:
                node_dict["alias"] = self._aliases[nid]
            node_dict["value"] = data["value"]
            node_dict["left"] = data["left"]
            node_dict["right"] = data["right"]
            nodes_list.append(node_dict)
        return {"root": self._root, "nodes": nodes_list}

    def _find_parent(self, node_id: int) -> int:
        r"""
        查找指定节点的父节点 ID

        :param node_id: 目标节点 ID
        :return int: 父节点 ID, 若为根节点或不存在则返回 -1
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
        new_id: int = self._next_id
        self._next_id += 1
        self._nodes[new_id] = {"value": value, "left": -1, "right": -1}
        self._root = new_id
        self._record_step("insert_root", {"value": value}, ret=new_id)
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
            self._raise_error(
                f"Left child already exists for node: {parent_id}")
        new_id: int = self._next_id
        self._next_id += 1
        self._nodes[new_id] = {"value": value, "left": -1, "right": -1}
        self._nodes[parent_id]["left"] = new_id
        self._record_step(
            "insert_left", {"parent": parent_id, "value": value}, ret=new_id)
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
            self._raise_error(
                f"Right child already exists for node: {parent_id}")
        new_id: int = self._next_id
        self._next_id += 1
        self._nodes[new_id] = {"value": value, "left": -1, "right": -1}
        self._nodes[parent_id]["right"] = new_id
        self._record_step(
            "insert_right", {"parent": parent_id, "value": value}, ret=new_id)
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
            self._aliases.pop(nid, None)
        self._record_step("delete", {"node_id": node_id})

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
        old_value: ValueType = self._nodes[node_id]["value"]
        self._nodes[node_id]["value"] = value
        self._record_step(
            "update_value", {"node_id": node_id, "value": value}, ret=old_value)


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

    def _node_exists(self, node_id: int) -> bool:
        r"""
        检查节点是否存在

        :param node_id: 节点 ID
        :return bool: 节点是否存在
        """
        return node_id in self._nodes

    def _build_data(self) -> dict[str, Any]:
        r"""
        构建二叉搜索树的状态数据

        :return dict[str, Any]: 状态数据字典
        """
        nodes_list: list[dict[str, Any]] = []
        for nid, data in sorted(self._nodes.items()):
            node_dict: dict[str, Any] = {"id": nid}
            if nid in self._aliases:
                node_dict["alias"] = self._aliases[nid]
            node_dict["value"] = data["value"]
            node_dict["left"] = data["left"]
            node_dict["right"] = data["right"]
            nodes_list.append(node_dict)
        return {"root": self._root, "nodes": nodes_list}

    def _find_node_by_value(self, value: int | float) -> int:
        r"""
        根据值查找节点 ID

        :param value: 要查找的值
        :return int: 节点 ID, 若不存在则返回 -1
        """
        for nid, data in self._nodes.items():
            if data["value"] == value:
                return nid
        return -1

    def _find_parent(self, node_id: int) -> int:
        r"""
        查找指定节点的父节点 ID

        :param node_id: 目标节点 ID
        :return int: 父节点 ID, 若为根节点或不存在则返回 -1
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
        插入节点, 自动维护二叉搜索树性质

        :param value: 要插入的值
        :return int: 新插入节点的 id
        """
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
        self._record_step("insert", {"value": value}, ret=new_id)
        return new_id

    def delete(self, value: int | float) -> None:
        r"""
        删除节点, 自动维护二叉搜索树性质

        :param value: 要删除的节点值
        :return None: 无返回值
        :raise RuntimeError: 当值不存在时抛出异常
        """
        node_id: int = self._find_node_by_value(value)
        if node_id == -1:
            self._raise_error(f"Node with value {value} not found")
        self._delete_node(node_id)
        self._record_step("delete", {"value": value})

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
            self._aliases.pop(node_id, None)
        elif left_child == -1:
            self._replace_in_parent(parent_id, node_id, right_child)
            del self._nodes[node_id]
            self._aliases.pop(node_id, None)
        elif right_child == -1:
            self._replace_in_parent(parent_id, node_id, left_child)
            del self._nodes[node_id]
            self._aliases.pop(node_id, None)
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
