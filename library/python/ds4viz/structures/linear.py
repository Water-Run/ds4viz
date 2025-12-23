r"""
线性数据结构实现：栈、队列、单链表、双向链表

:file: ds4viz/structures/linear.py
:author: WaterRun
:time: 2025-12-23
"""

from typing import Any

from ds4viz.structures._base import BaseStructure, ValueType
from ds4viz.session import get_caller_line


class Stack(BaseStructure):
    r"""
    栈数据结构
    """

    def __init__(self, label: str = "stack", output: str | None = None) -> None:
        r"""
        初始化栈

        :param label: 栈的标签
        :param output: 输出文件路径
        :return None: 无返回值
        """
        super().__init__("stack", label, output)
        self._items: list[ValueType] = []

    def _build_data(self) -> dict[str, Any]:
        r"""
        构建栈的状态数据

        :return dict[str, Any]: 状态数据字典
        """
        return {
            "items": list(self._items),
            "top": len(self._items) - 1 if self._items else -1
        }

    def push(self, value: ValueType) -> None:
        r"""
        压栈操作

        :param value: 要压入的值
        :return None: 无返回值
        """
        before: int = self._session.get_last_state_id()
        line: int = get_caller_line(2)
        self._items.append(value)
        after: int = self._session.add_state(self._build_data())
        self._session.add_step(
            op="push",
            before=before,
            after=after,
            args={"value": value},
            line=line
        )

    def pop(self) -> None:
        r"""
        弹栈操作

        :return None: 无返回值
        :raise RuntimeError: 当栈为空时抛出异常
        """
        if not self._items:
            self._raise_error("Cannot pop from empty stack")
        before: int = self._session.get_last_state_id()
        line: int = get_caller_line(2)
        popped: ValueType = self._items.pop()
        after: int = self._session.add_state(self._build_data())
        self._session.add_step(
            op="pop",
            before=before,
            after=after,
            args={},
            ret=popped,
            line=line
        )

    def clear(self) -> None:
        r"""
        清空栈

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


class Queue(BaseStructure):
    r"""
    队列数据结构
    """

    def __init__(self, label: str = "queue", output: str | None = None) -> None:
        r"""
        初始化队列

        :param label: 队列的标签
        :param output: 输出文件路径
        :return None: 无返回值
        """
        super().__init__("queue", label, output)
        self._items: list[ValueType] = []

    def _build_data(self) -> dict[str, Any]:
        r"""
        构建队列的状态数据

        :return dict[str, Any]: 状态数据字典
        """
        if not self._items:
            return {"items": [], "front": -1, "rear": -1}
        return {
            "items": list(self._items),
            "front": 0,
            "rear": len(self._items) - 1
        }

    def enqueue(self, value: ValueType) -> None:
        r"""
        入队操作

        :param value: 要入队的值
        :return None: 无返回值
        """
        before: int = self._session.get_last_state_id()
        line: int = get_caller_line(2)
        self._items.append(value)
        after: int = self._session.add_state(self._build_data())
        self._session.add_step(
            op="enqueue",
            before=before,
            after=after,
            args={"value": value},
            line=line
        )

    def dequeue(self) -> None:
        r"""
        出队操作

        :return None: 无返回值
        :raise RuntimeError: 当队列为空时抛出异常
        """
        if not self._items:
            self._raise_error("Cannot dequeue from empty queue")
        before: int = self._session.get_last_state_id()
        line: int = get_caller_line(2)
        dequeued: ValueType = self._items.pop(0)
        after: int = self._session.add_state(self._build_data())
        self._session.add_step(
            op="dequeue",
            before=before,
            after=after,
            args={},
            ret=dequeued,
            line=line
        )

    def clear(self) -> None:
        r"""
        清空队列

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


class SingleLinkedList(BaseStructure):
    r"""
    单链表数据结构
    """

    def __init__(self, label: str = "slist", output: str | None = None) -> None:
        r"""
        初始化单链表

        :param label: 单链表的标签
        :param output: 输出文件路径
        :return None: 无返回值
        """
        super().__init__("slist", label, output)
        self._nodes: dict[int, dict[str, Any]] = {}
        self._head: int = -1
        self._next_id: int = 0

    def _build_data(self) -> dict[str, Any]:
        r"""
        构建单链表的状态数据

        :return dict[str, Any]: 状态数据字典
        """
        nodes_list: list[dict[str, Any]] = [
            {"id": nid, "value": data["value"], "next": data["next"]}
            for nid, data in sorted(self._nodes.items())
        ]
        return {"head": self._head, "nodes": nodes_list}

    def _find_tail(self) -> int:
        r"""
        查找尾节点 ID

        :return int: 尾节点 ID，若链表为空则返回 -1
        """
        if self._head == -1:
            return -1
        current: int = self._head
        while self._nodes[current]["next"] != -1:
            current = self._nodes[current]["next"]
        return current

    def _find_prev(self, node_id: int) -> int:
        r"""
        查找指定节点的前驱节点 ID

        :param node_id: 目标节点 ID
        :return int: 前驱节点 ID，若为头节点或不存在则返回 -1
        """
        if self._head == -1 or self._head == node_id:
            return -1
        current: int = self._head
        while current != -1:
            if self._nodes[current]["next"] == node_id:
                return current
            current = self._nodes[current]["next"]
        return -1

    def insert_head(self, value: ValueType) -> int:
        r"""
        在链表头部插入节点

        :param value: 要插入的值
        :return int: 新插入节点的 id
        """
        before: int = self._session.get_last_state_id()
        line: int = get_caller_line(2)
        new_id: int = self._next_id
        self._next_id += 1
        self._nodes[new_id] = {"value": value, "next": self._head}
        self._head = new_id
        after: int = self._session.add_state(self._build_data())
        self._session.add_step(
            op="insert_head",
            before=before,
            after=after,
            args={"value": value},
            ret=new_id,
            line=line
        )
        return new_id

    def insert_tail(self, value: ValueType) -> int:
        r"""
        在链表尾部插入节点

        :param value: 要插入的值
        :return int: 新插入节点的 id
        """
        before: int = self._session.get_last_state_id()
        line: int = get_caller_line(2)
        new_id: int = self._next_id
        self._next_id += 1
        self._nodes[new_id] = {"value": value, "next": -1}
        if self._head == -1:
            self._head = new_id
        else:
            tail: int = self._find_tail()
            self._nodes[tail]["next"] = new_id
        after: int = self._session.add_state(self._build_data())
        self._session.add_step(
            op="insert_tail",
            before=before,
            after=after,
            args={"value": value},
            ret=new_id,
            line=line
        )
        return new_id

    def insert_after(self, node_id: int, value: ValueType) -> int:
        r"""
        在指定节点后插入新节点

        :param node_id: 目标节点的 id
        :param value: 要插入的值
        :return int: 新插入节点的 id
        :raise RuntimeError: 当指定节点不存在时抛出异常
        """
        if node_id not in self._nodes:
            self._raise_error(f"Node not found: {node_id}")
        before: int = self._session.get_last_state_id()
        line: int = get_caller_line(2)
        new_id: int = self._next_id
        self._next_id += 1
        old_next: int = self._nodes[node_id]["next"]
        self._nodes[new_id] = {"value": value, "next": old_next}
        self._nodes[node_id]["next"] = new_id
        after: int = self._session.add_state(self._build_data())
        self._session.add_step(
            op="insert_after",
            before=before,
            after=after,
            args={"node_id": node_id, "value": value},
            ret=new_id,
            line=line
        )
        return new_id

    def delete(self, node_id: int) -> None:
        r"""
        删除指定节点

        :param node_id: 要删除的节点 id
        :return None: 无返回值
        :raise RuntimeError: 当指定节点不存在时抛出异常
        """
        if node_id not in self._nodes:
            self._raise_error(f"Node not found: {node_id}")
        before: int = self._session.get_last_state_id()
        line: int = get_caller_line(2)
        deleted_value: ValueType = self._nodes[node_id]["value"]
        if self._head == node_id:
            self._head = self._nodes[node_id]["next"]
        else:
            prev: int = self._find_prev(node_id)
            if prev != -1:
                self._nodes[prev]["next"] = self._nodes[node_id]["next"]
        del self._nodes[node_id]
        after: int = self._session.add_state(self._build_data())
        self._session.add_step(
            op="delete",
            before=before,
            after=after,
            args={"node_id": node_id},
            ret=deleted_value,
            line=line
        )

    def delete_head(self) -> None:
        r"""
        删除头节点

        :return None: 无返回值
        :raise RuntimeError: 当链表为空时抛出异常
        """
        if self._head == -1:
            self._raise_error("Cannot delete from empty list")
        before: int = self._session.get_last_state_id()
        line: int = get_caller_line(2)
        old_head: int = self._head
        deleted_value: ValueType = self._nodes[old_head]["value"]
        self._head = self._nodes[old_head]["next"]
        del self._nodes[old_head]
        after: int = self._session.add_state(self._build_data())
        self._session.add_step(
            op="delete_head",
            before=before,
            after=after,
            args={},
            ret=deleted_value,
            line=line
        )

    def reverse(self) -> None:
        r"""
        反转链表

        :return None: 无返回值
        """
        before: int = self._session.get_last_state_id()
        line: int = get_caller_line(2)
        prev: int = -1
        current: int = self._head
        while current != -1:
            next_node: int = self._nodes[current]["next"]
            self._nodes[current]["next"] = prev
            prev = current
            current = next_node
        self._head = prev
        after: int = self._session.add_state(self._build_data())
        self._session.add_step(
            op="reverse",
            before=before,
            after=after,
            args={},
            line=line
        )


class DoubleLinkedList(BaseStructure):
    r"""
    双向链表数据结构
    """

    def __init__(self, label: str = "dlist", output: str | None = None) -> None:
        r"""
        初始化双向链表

        :param label: 双向链表的标签
        :param output: 输出文件路径
        :return None: 无返回值
        """
        super().__init__("dlist", label, output)
        self._nodes: dict[int, dict[str, Any]] = {}
        self._head: int = -1
        self._tail: int = -1
        self._next_id: int = 0

    def _build_data(self) -> dict[str, Any]:
        r"""
        构建双向链表的状态数据

        :return dict[str, Any]: 状态数据字典
        """
        nodes_list: list[dict[str, Any]] = [
            {"id": nid, "value": data["value"], "prev": data["prev"], "next": data["next"]}
            for nid, data in sorted(self._nodes.items())
        ]
        return {"head": self._head, "tail": self._tail, "nodes": nodes_list}

    def insert_head(self, value: ValueType) -> int:
        r"""
        在链表头部插入节点

        :param value: 要插入的值
        :return int: 新插入节点的 id
        """
        before: int = self._session.get_last_state_id()
        line: int = get_caller_line(2)
        new_id: int = self._next_id
        self._next_id += 1
        self._nodes[new_id] = {"value": value, "prev": -1, "next": self._head}
        if self._head != -1:
            self._nodes[self._head]["prev"] = new_id
        self._head = new_id
        if self._tail == -1:
            self._tail = new_id
        after: int = self._session.add_state(self._build_data())
        self._session.add_step(
            op="insert_head",
            before=before,
            after=after,
            args={"value": value},
            ret=new_id,
            line=line
        )
        return new_id

    def insert_tail(self, value: ValueType) -> int:
        r"""
        在链表尾部插入节点

        :param value: 要插入的值
        :return int: 新插入节点的 id
        """
        before: int = self._session.get_last_state_id()
        line: int = get_caller_line(2)
        new_id: int = self._next_id
        self._next_id += 1
        self._nodes[new_id] = {"value": value, "prev": self._tail, "next": -1}
        if self._tail != -1:
            self._nodes[self._tail]["next"] = new_id
        self._tail = new_id
        if self._head == -1:
            self._head = new_id
        after: int = self._session.add_state(self._build_data())
        self._session.add_step(
            op="insert_tail",
            before=before,
            after=after,
            args={"value": value},
            ret=new_id,
            line=line
        )
        return new_id

    def insert_before(self, node_id: int, value: ValueType) -> int:
        r"""
        在指定节点前插入新节点

        :param node_id: 目标节点的 id
        :param value: 要插入的值
        :return int: 新插入节点的 id
        :raise RuntimeError: 当指定节点不存在时抛出异常
        """
        if node_id not in self._nodes:
            self._raise_error(f"Node not found: {node_id}")
        before: int = self._session.get_last_state_id()
        line: int = get_caller_line(2)
        new_id: int = self._next_id
        self._next_id += 1
        old_prev: int = self._nodes[node_id]["prev"]
        self._nodes[new_id] = {"value": value, "prev": old_prev, "next": node_id}
        self._nodes[node_id]["prev"] = new_id
        if old_prev != -1:
            self._nodes[old_prev]["next"] = new_id
        else:
            self._head = new_id
        after: int = self._session.add_state(self._build_data())
        self._session.add_step(
            op="insert_before",
            before=before,
            after=after,
            args={"node_id": node_id, "value": value},
            ret=new_id,
            line=line
        )
        return new_id

    def insert_after(self, node_id: int, value: ValueType) -> int:
        r"""
        在指定节点后插入新节点

        :param node_id: 目标节点的 id
        :param value: 要插入的值
        :return int: 新插入节点的 id
        :raise RuntimeError: 当指定节点不存在时抛出异常
        """
        if node_id not in self._nodes:
            self._raise_error(f"Node not found: {node_id}")
        before: int = self._session.get_last_state_id()
        line: int = get_caller_line(2)
        new_id: int = self._next_id
        self._next_id += 1
        old_next: int = self._nodes[node_id]["next"]
        self._nodes[new_id] = {"value": value, "prev": node_id, "next": old_next}
        self._nodes[node_id]["next"] = new_id
        if old_next != -1:
            self._nodes[old_next]["prev"] = new_id
        else:
            self._tail = new_id
        after: int = self._session.add_state(self._build_data())
        self._session.add_step(
            op="insert_after",
            before=before,
            after=after,
            args={"node_id": node_id, "value": value},
            ret=new_id,
            line=line
        )
        return new_id

    def delete(self, node_id: int) -> None:
        r"""
        删除指定节点

        :param node_id: 要删除的节点 id
        :return None: 无返回值
        :raise RuntimeError: 当指定节点不存在时抛出异常
        """
        if node_id not in self._nodes:
            self._raise_error(f"Node not found: {node_id}")
        before: int = self._session.get_last_state_id()
        line: int = get_caller_line(2)
        deleted_value: ValueType = self._nodes[node_id]["value"]
        prev_id: int = self._nodes[node_id]["prev"]
        next_id: int = self._nodes[node_id]["next"]
        if prev_id != -1:
            self._nodes[prev_id]["next"] = next_id
        else:
            self._head = next_id
        if next_id != -1:
            self._nodes[next_id]["prev"] = prev_id
        else:
            self._tail = prev_id
        del self._nodes[node_id]
        after: int = self._session.add_state(self._build_data())
        self._session.add_step(
            op="delete",
            before=before,
            after=after,
            args={"node_id": node_id},
            ret=deleted_value,
            line=line
        )

    def delete_head(self) -> None:
        r"""
        删除头节点

        :return None: 无返回值
        :raise RuntimeError: 当链表为空时抛出异常
        """
        if self._head == -1:
            self._raise_error("Cannot delete from empty list")
        self.delete(self._head)

    def delete_tail(self) -> None:
        r"""
        删除尾节点

        :return None: 无返回值
        :raise RuntimeError: 当链表为空时抛出异常
        """
        if self._tail == -1:
            self._raise_error("Cannot delete from empty list")
        self.delete(self._tail)

    def reverse(self) -> None:
        r"""
        反转链表

        :return None: 无返回值
        """
        before: int = self._session.get_last_state_id()
        line: int = get_caller_line(2)
        current: int = self._head
        while current != -1:
            node: dict[str, Any] = self._nodes[current]
            node["prev"], node["next"] = node["next"], node["prev"]
            current = node["prev"]
        self._head, self._tail = self._tail, self._head
        after: int = self._session.add_state(self._build_data())
        self._session.add_step(
            op="reverse",
            before=before,
            after=after,
            args={},
            line=line
        )
