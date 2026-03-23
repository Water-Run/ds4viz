r"""
ds4viz - 可扩展的数据结构可视化教学平台 Python 库

提供数据结构的可视化追踪功能, 生成标准化的 TOML 中间表示文件.

:file: ds4viz/__init__.py
:author: WaterRun
:time: 2026-03-23
"""

from typing import Any

from ds4viz.session import config
from ds4viz.structures.linear import Stack, Queue, SingleLinkedList, DoubleLinkedList
from ds4viz.structures.tree import BinaryTree, BinarySearchTree
from ds4viz.structures.graph import GraphUndirected, GraphDirected, GraphWeighted


def node(target: int, style: str = "focus", level: int = 1) -> dict[str, Any]:
    r"""
    创建节点高亮标记

    :param target: 节点 id
    :param style: 高亮样式, 长度 1..32
    :param level: 视觉强度等级, 1..9, 默认为 1
    :return dict[str, Any]: 高亮标记字典
    """
    return {"kind": "node", "target": target, "style": style, "level": level}


def item(target: int, style: str = "focus", level: int = 1) -> dict[str, Any]:
    r"""
    创建元素高亮标记, 仅适用于 stack 和 queue

    :param target: 元素在 items 数组中的索引
    :param style: 高亮样式, 长度 1..32
    :param level: 视觉强度等级, 1..9, 默认为 1
    :return dict[str, Any]: 高亮标记字典
    """
    return {"kind": "item", "target": target, "style": style, "level": level}


def edge(from_id: int, to_id: int, style: str = "focus", level: int = 1) -> dict[str, Any]:
    r"""
    创建边高亮标记, 仅适用于图结构

    :param from_id: 起始节点 id
    :param to_id: 终止节点 id
    :param style: 高亮样式, 长度 1..32
    :param level: 视觉强度等级, 1..9, 默认为 1
    :return dict[str, Any]: 高亮标记字典
    """
    return {"kind": "edge", "from": from_id, "to": to_id, "style": style, "level": level}


def stack(label: str = "stack", output: str | None = None) -> Stack:
    r"""
    创建栈实例

    :param label: 栈的标签, 默认为 "stack"
    :param output: 输出文件路径, 若为 None 则使用全局配置
    :return Stack: 栈实例
    """
    return Stack(label=label, output=output)


def queue(label: str = "queue", output: str | None = None) -> Queue:
    r"""
    创建队列实例

    :param label: 队列的标签, 默认为 "queue"
    :param output: 输出文件路径, 若为 None 则使用全局配置
    :return Queue: 队列实例
    """
    return Queue(label=label, output=output)


def single_linked_list(label: str = "slist", output: str | None = None) -> SingleLinkedList:
    r"""
    创建单链表实例

    :param label: 单链表的标签, 默认为 "slist"
    :param output: 输出文件路径, 若为 None 则使用全局配置
    :return SingleLinkedList: 单链表实例
    """
    return SingleLinkedList(label=label, output=output)


def double_linked_list(label: str = "dlist", output: str | None = None) -> DoubleLinkedList:
    r"""
    创建双向链表实例

    :param label: 双向链表的标签, 默认为 "dlist"
    :param output: 输出文件路径, 若为 None 则使用全局配置
    :return DoubleLinkedList: 双向链表实例
    """
    return DoubleLinkedList(label=label, output=output)


def binary_tree(label: str = "binary_tree", output: str | None = None) -> BinaryTree:
    r"""
    创建二叉树实例

    :param label: 二叉树的标签, 默认为 "binary_tree"
    :param output: 输出文件路径, 若为 None 则使用全局配置
    :return BinaryTree: 二叉树实例
    """
    return BinaryTree(label=label, output=output)


def binary_search_tree(label: str = "bst", output: str | None = None) -> BinarySearchTree:
    r"""
    创建二叉搜索树实例

    :param label: 二叉搜索树的标签, 默认为 "bst"
    :param output: 输出文件路径, 若为 None 则使用全局配置
    :return BinarySearchTree: 二叉搜索树实例
    """
    return BinarySearchTree(label=label, output=output)


def graph_undirected(label: str = "graph", output: str | None = None) -> GraphUndirected:
    r"""
    创建无向图实例

    :param label: 无向图的标签, 默认为 "graph"
    :param output: 输出文件路径, 若为 None 则使用全局配置
    :return GraphUndirected: 无向图实例
    """
    return GraphUndirected(label=label, output=output)


def graph_directed(label: str = "graph", output: str | None = None) -> GraphDirected:
    r"""
    创建有向图实例

    :param label: 有向图的标签, 默认为 "graph"
    :param output: 输出文件路径, 若为 None 则使用全局配置
    :return GraphDirected: 有向图实例
    """
    return GraphDirected(label=label, output=output)


def graph_weighted(label: str = "graph", output: str | None = None) -> GraphWeighted:
    r"""
    创建带权有向图实例

    :param label: 带权图的标签, 默认为 "graph"
    :param output: 输出文件路径, 若为 None 则使用全局配置
    :return GraphWeighted: 带权有向图实例
    """
    return GraphWeighted(label=label, output=output)


__version__ = "0.1.0"
__author__ = "WaterRun <linzhuangrun49@gmail.com>"

__all__ = [
    "config",
    "node",
    "item",
    "edge",
    "stack",
    "queue",
    "single_linked_list",
    "double_linked_list",
    "binary_tree",
    "binary_search_tree",
    "graph_undirected",
    "graph_directed",
    "graph_weighted",
    "Stack",
    "Queue",
    "SingleLinkedList",
    "DoubleLinkedList",
    "BinaryTree",
    "BinarySearchTree",
    "GraphUndirected",
    "GraphDirected",
    "GraphWeighted",
]
