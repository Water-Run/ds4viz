r"""
集成测试模块 - 完整的端到端测试

:file: test/intergration_test.py
:author: WaterRun
:time: 2026-03-27
"""

import re
from pathlib import Path
import inspect
import tomllib
import pytest

import ds4viz
from ds4viz import (
    config, stack, queue, single_linked_list, double_linked_list,
    binary_tree, binary_search_tree,
    graph_undirected, graph_directed, graph_weighted,
)
from ds4viz.exception import StructureError


class TomlValidator:
    r"""TOML IR 验证器"""

    VALID_KINDS: set[str] = {
        "stack", "queue", "slist", "dlist",
        "binary_tree", "bst",
        "graph_undirected", "graph_directed", "graph_weighted"
    }
    VALID_ERROR_TYPES: set[str] = {
        "runtime", "timeout", "validation", "sandbox", "unknown"}
    VALID_LANGS: set[str] = {"python", "c", "zig",
                             "rust", "java", "csharp", "typescript", "lua"}

    def __init__(self, content: str) -> None:
        r"""
        初始化验证器

        :param content: TOML 文件内容
        :return None: 无返回值
        """
        self.content: str = content
        self.errors: list[str] = []

    def validate_all(self) -> bool:
        r"""
        执行所有验证

        :return bool: 是否通过所有验证
        """
        self._validate_meta()
        self._validate_package()
        self._validate_object()
        self._validate_states()
        self._validate_steps()
        self._validate_result_or_error()
        return len(self.errors) == 0

    def _validate_meta(self) -> None:
        r"""
        验证 [meta] 部分

        :return None: 无返回值
        """
        if "[meta]" not in self.content:
            self.errors.append("Missing [meta]")
            return
        if not re.search(r'created_at\s*=\s*"\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}Z"', self.content):
            self.errors.append("Invalid meta.created_at")
        lang_match = re.search(
            r'\[meta].*?lang\s*=\s*"([^"]+)"', self.content, re.DOTALL)
        if lang_match and lang_match.group(1) not in self.VALID_LANGS:
            self.errors.append(f"Invalid meta.lang: {lang_match.group(1)}")

    def _validate_package(self) -> None:
        r"""
        验证 [package] 部分

        :return None: 无返回值
        """
        if "[package]" not in self.content:
            self.errors.append("Missing [package]")
            return
        if not re.search(r'\[package].*?version\s*=\s*"\d+\.\d+\.\d+"', self.content, re.DOTALL):
            self.errors.append("Invalid package.version")

    def _validate_object(self) -> None:
        r"""
        验证 [object] 部分

        :return None: 无返回值
        """
        if "[object]" not in self.content:
            self.errors.append("Missing [object]")
            return
        kind_match = re.search(
            r'\[object].*?kind\s*=\s*"([^"]+)"', self.content, re.DOTALL)
        if kind_match and kind_match.group(1) not in self.VALID_KINDS:
            self.errors.append(f"Invalid object.kind: {kind_match.group(1)}")

    def _validate_states(self) -> None:
        r"""
        验证 [[states]] 部分

        :return None: 无返回值
        """
        if "[[states]]" not in self.content:
            self.errors.append("Missing [[states]]")
            return
        state_ids: list[int] = [int(m) for m in re.findall(
            r"\[\[states]]\s*\nid\s*=\s*(\d+)", self.content)]
        if state_ids and state_ids != list(range(len(state_ids))):
            self.errors.append(f"State IDs not consecutive: {state_ids}")

    def _validate_steps(self) -> None:
        r"""
        验证 [[steps]] 部分

        :return None: 无返回值
        """
        step_ids: list[int] = [int(m) for m in re.findall(
            r"\[\[steps]]\s*\nid\s*=\s*(\d+)", self.content)]
        if step_ids and step_ids != list(range(len(step_ids))):
            self.errors.append(f"Step IDs not consecutive: {step_ids}")

    def _validate_result_or_error(self) -> None:
        r"""
        验证 [result] / [error] 互斥

        :return None: 无返回值
        """
        has_result: bool = "[result]" in self.content
        has_error: bool = "[error]" in self.content
        if has_result and has_error:
            self.errors.append("Both [result] and [error] present")
        if not has_result and not has_error:
            self.errors.append("Neither [result] nor [error] present")


def _validate(content: str) -> None:
    r"""
    验证 TOML 内容并在失败时断言

    :param content: TOML 内容
    :return None: 无返回值
    """
    v = TomlValidator(content)
    assert v.validate_all(), f"Validation errors: {v.errors}"


class TestStackIntegration:
    r"""栈集成测试"""

    def test_basic_operations(self, tmp_path: Path) -> None:
        r"""测试栈基本操作"""
        out = tmp_path / "s.toml"
        with stack(label="test", output=str(out)) as s:
            s.push(10)
            s.push(20)
            val = s.pop()
            assert val == 20
            s.push(30)
        _validate(out.read_text(encoding="utf-8"))

    def test_empty_pop_error(self, tmp_path: Path) -> None:
        r"""测试空栈弹出错误"""
        out = tmp_path / "s.toml"
        with pytest.raises(StructureError):
            with stack(output=str(out)) as s:
                s.pop()
        content: str = out.read_text(encoding="utf-8")
        _validate(content)
        assert "[error]" in content

    def test_clear(self, tmp_path: Path) -> None:
        r"""测试清空"""
        out = tmp_path / "s.toml"
        with stack(output=str(out)) as s:
            s.push(1)
            s.push(2)
            s.clear()
        content: str = out.read_text(encoding="utf-8")
        _validate(content)
        assert "items = []" in content

    def test_various_types(self, tmp_path: Path) -> None:
        r"""测试各种值类型"""
        out = tmp_path / "s.toml"
        with stack(output=str(out)) as s:
            s.push(42)
            s.push(3.14)
            s.push("hello")
            s.push(True)
        _validate(out.read_text(encoding="utf-8"))

    def test_step_and_amend(self, tmp_path: Path) -> None:
        r"""测试 step 和 amend"""
        out = tmp_path / "s.toml"
        with stack(output=str(out)) as s:
            s.push(10)
            s.amend(note="压入 10", highlights=[
                    ds4viz.item(0, "focus", level=3)])
            s.push(20)
            s.step(note="查看栈", highlights=[
                ds4viz.item(0, "visited"),
                ds4viz.item(1, "focus", level=3)
            ])
        content: str = out.read_text(encoding="utf-8")
        _validate(content)
        assert 'note = "压入 10"' in content
        assert 'op = "observe"' in content

    def test_phase(self, tmp_path: Path) -> None:
        r"""测试 phase"""
        out = tmp_path / "s.toml"
        with stack(output=str(out)) as s:
            with s.phase("build"):
                s.push(10)
                s.push(20)
            with s.phase("pop"):
                s.pop()
        content: str = out.read_text(encoding="utf-8")
        _validate(content)
        assert 'phase = "build"' in content
        assert 'phase = "pop"' in content

    def test_push_pop_push_cycle(self, tmp_path: Path) -> None:
        r"""测试 push-pop-push 循环操作"""
        out = tmp_path / "s.toml"
        with stack(output=str(out)) as s:
            s.push(10)
            s.push(20)
            val = s.pop()
            assert val == 20
            s.push(30)
            s.push(40)
            val = s.pop()
            assert val == 40
            val = s.pop()
            assert val == 30
        _validate(out.read_text(encoding="utf-8"))

    def test_sequential_pop_returns_lifo_order(self, tmp_path: Path) -> None:
        r"""测试连续弹出遵循 LIFO 顺序"""
        out = tmp_path / "s.toml"
        with stack(output=str(out)) as s:
            for v in [1, 2, 3, 4, 5]:
                s.push(v)
            results: list = []
            for _ in range(5):
                results.append(s.pop())
            assert results == [5, 4, 3, 2, 1]
        _validate(out.read_text(encoding="utf-8"))

    def test_amend_without_previous_step_error(self, tmp_path: Path) -> None:
        r"""测试无前置步骤时 amend 报错"""
        out = tmp_path / "s.toml"
        with pytest.raises((StructureError, RuntimeError)):
            with stack(output=str(out)) as s:
                s.amend(note="no previous step")

    def test_nested_phases(self, tmp_path: Path) -> None:
        r"""测试嵌套 phase, 内层优先"""
        out = tmp_path / "s.toml"
        with stack(output=str(out)) as s:
            with s.phase("outer"):
                s.push(10)
                with s.phase("inner"):
                    s.push(20)
                s.push(30)
        content: str = out.read_text(encoding="utf-8")
        _validate(content)
        assert 'phase = "inner"' in content
        assert 'phase = "outer"' in content

    def test_clear_then_push(self, tmp_path: Path) -> None:
        r"""测试清空后继续操作"""
        out = tmp_path / "s.toml"
        with stack(output=str(out)) as s:
            s.push(1)
            s.push(2)
            s.clear()
            s.push(3)
            val = s.pop()
            assert val == 3
        _validate(out.read_text(encoding="utf-8"))

    def test_single_push_pop(self, tmp_path: Path) -> None:
        r"""测试单次 push 和 pop"""
        out = tmp_path / "s.toml"
        with stack(output=str(out)) as s:
            s.push(42)
            val = s.pop()
            assert val == 42
        content: str = out.read_text(encoding="utf-8")
        _validate(content)
        assert "[result]" in content

    def test_pop_after_clear_error(self, tmp_path: Path) -> None:
        r"""测试清空后弹出报错"""
        out = tmp_path / "s.toml"
        with pytest.raises(StructureError):
            with stack(output=str(out)) as s:
                s.push(1)
                s.clear()
                s.pop()
        content: str = out.read_text(encoding="utf-8")
        _validate(content)
        assert "[error]" in content


class TestQueueIntegration:
    r"""队列集成测试"""

    def test_basic_operations(self, tmp_path: Path) -> None:
        r"""测试队列基本操作"""
        out = tmp_path / "q.toml"
        with queue(output=str(out)) as q:
            q.enqueue(10)
            q.enqueue(20)
            val = q.dequeue()
            assert val == 10
        _validate(out.read_text(encoding="utf-8"))

    def test_empty_dequeue_error(self, tmp_path: Path) -> None:
        r"""测试空队列出队错误"""
        out = tmp_path / "q.toml"
        with pytest.raises(StructureError):
            with queue(output=str(out)) as q:
                q.dequeue()
        assert "[error]" in out.read_text(encoding="utf-8")

    def test_phase_and_amend(self, tmp_path: Path) -> None:
        r"""测试队列 phase 和 amend"""
        out = tmp_path / "q.toml"
        with queue(output=str(out)) as q:
            with q.phase("入队"):
                q.enqueue("甲")
                q.enqueue("乙")
            with q.phase("处理"):
                val = q.dequeue()
                q.amend(note=f"处理 {val}")
        content: str = out.read_text(encoding="utf-8")
        _validate(content)
        assert 'phase = "入队"' in content
        assert 'note = "处理 甲"' in content

    def test_clear(self, tmp_path: Path) -> None:
        r"""测试队列清空"""
        out = tmp_path / "q.toml"
        with queue(output=str(out)) as q:
            q.enqueue(1)
            q.enqueue(2)
            q.enqueue(3)
            q.clear()
        content: str = out.read_text(encoding="utf-8")
        _validate(content)
        assert "items = []" in content

    def test_various_types(self, tmp_path: Path) -> None:
        r"""测试队列各种值类型"""
        out = tmp_path / "q.toml"
        with queue(output=str(out)) as q:
            q.enqueue(42)
            q.enqueue(2.718)
            q.enqueue("world")
            q.enqueue(False)
        _validate(out.read_text(encoding="utf-8"))

    def test_fifo_order(self, tmp_path: Path) -> None:
        r"""测试出队遵循 FIFO 顺序"""
        out = tmp_path / "q.toml"
        with queue(output=str(out)) as q:
            for v in ["a", "b", "c", "d"]:
                q.enqueue(v)
            results: list = []
            for _ in range(4):
                results.append(q.dequeue())
            assert results == ["a", "b", "c", "d"]
        _validate(out.read_text(encoding="utf-8"))

    def test_step_with_highlights(self, tmp_path: Path) -> None:
        r"""测试队列 step 与高亮"""
        out = tmp_path / "q.toml"
        with queue(output=str(out)) as q:
            q.enqueue(10)
            q.enqueue(20)
            q.step(note="观察队列", highlights=[
                ds4viz.item(0, "focus", level=3),
                ds4viz.item(1, "visited")
            ])
        content: str = out.read_text(encoding="utf-8")
        _validate(content)
        assert 'op = "observe"' in content
        assert 'note = "观察队列"' in content

    def test_dequeue_after_clear_error(self, tmp_path: Path) -> None:
        r"""测试清空后出队报错"""
        out = tmp_path / "q.toml"
        with pytest.raises(StructureError):
            with queue(output=str(out)) as q:
                q.enqueue(1)
                q.clear()
                q.dequeue()
        content: str = out.read_text(encoding="utf-8")
        _validate(content)
        assert "[error]" in content


class TestSingleLinkedListIntegration:
    r"""单链表集成测试"""

    def test_basic_operations(self, tmp_path: Path) -> None:
        r"""测试单链表基本操作"""
        out = tmp_path / "sl.toml"
        with single_linked_list(output=str(out)) as slist:
            a: int = slist.insert_head(10)
            slist.alias(a, "head")
            slist.insert_tail(20)
            slist.insert_after(a, 15)
            slist.delete_head()
        content: str = out.read_text(encoding="utf-8")
        _validate(content)
        assert 'kind = "slist"' in content

    def test_reverse(self, tmp_path: Path) -> None:
        r"""测试反转"""
        out = tmp_path / "sl.toml"
        with single_linked_list(output=str(out)) as slist:
            slist.insert_tail(1)
            slist.insert_tail(2)
            slist.insert_tail(3)
            slist.reverse()
        _validate(out.read_text(encoding="utf-8"))

    def test_delete_nonexistent_error(self, tmp_path: Path) -> None:
        r"""测试删除不存在的节点"""
        out = tmp_path / "sl.toml"
        with pytest.raises(StructureError):
            with single_linked_list(output=str(out)) as slist:
                slist.insert_head(10)
                slist.delete(999)
        assert "[error]" in out.read_text(encoding="utf-8")

    def test_alias_and_step(self, tmp_path: Path) -> None:
        r"""测试别名和 step"""
        out = tmp_path / "sl.toml"
        with single_linked_list(output=str(out)) as slist:
            a: int = slist.insert_head(10)
            b: int = slist.insert_tail(20)
            slist.alias(a, "start")
            slist.step(note="遍历开始", highlights=[
                       ds4viz.node(a, "focus", level=3)])
            slist.step(note="遍历结束", highlights=[
                ds4viz.node(a, "visited"), ds4viz.node(b, "focus", level=3)
            ])
        content: str = out.read_text(encoding="utf-8")
        _validate(content)
        assert 'alias = "start"' in content
        assert 'op = "observe"' in content

    def test_delete_head_single_node(self, tmp_path: Path) -> None:
        r"""测试删除唯一节点的头"""
        out = tmp_path / "sl.toml"
        with single_linked_list(output=str(out)) as slist:
            slist.insert_head(10)
            slist.delete_head()
        content: str = out.read_text(encoding="utf-8")
        _validate(content)
        assert "head = -1" in content

    def test_insert_after_nonexistent_error(self, tmp_path: Path) -> None:
        r"""测试在不存在的节点后插入"""
        out = tmp_path / "sl.toml"
        with pytest.raises(StructureError):
            with single_linked_list(output=str(out)) as slist:
                slist.insert_head(10)
                slist.insert_after(999, 20)
        assert "[error]" in out.read_text(encoding="utf-8")

    def test_delete_head_empty_error(self, tmp_path: Path) -> None:
        r"""测试空链表删除头"""
        out = tmp_path / "sl.toml"
        with pytest.raises(StructureError):
            with single_linked_list(output=str(out)) as slist:
                slist.delete_head()
        content: str = out.read_text(encoding="utf-8")
        _validate(content)
        assert "[error]" in content

    def test_multiple_head_inserts(self, tmp_path: Path) -> None:
        r"""测试多次头插入"""
        out = tmp_path / "sl.toml"
        with single_linked_list(output=str(out)) as slist:
            a: int = slist.insert_head(30)
            b: int = slist.insert_head(20)
            c: int = slist.insert_head(10)
            slist.step(note="检查顺序", highlights=[
                ds4viz.node(c, "focus", level=3),
                ds4viz.node(b, "visited"),
                ds4viz.node(a, "visited")
            ])
        _validate(out.read_text(encoding="utf-8"))

    def test_reverse_single_node(self, tmp_path: Path) -> None:
        r"""测试单节点反转"""
        out = tmp_path / "sl.toml"
        with single_linked_list(output=str(out)) as slist:
            slist.insert_head(42)
            slist.reverse()
        _validate(out.read_text(encoding="utf-8"))

    def test_reverse_empty(self, tmp_path: Path) -> None:
        r"""测试空链表反转"""
        out = tmp_path / "sl.toml"
        with single_linked_list(output=str(out)) as slist:
            slist.reverse()
        _validate(out.read_text(encoding="utf-8"))


class TestDoubleLinkedListIntegration:
    r"""双向链表集成测试"""

    def test_basic_operations(self, tmp_path: Path) -> None:
        r"""测试双向链表基本操作"""
        out = tmp_path / "dl.toml"
        with double_linked_list(output=str(out)) as dlist:
            a: int = dlist.insert_head(10)
            b: int = dlist.insert_tail(30)
            dlist.insert_before(b, 20)
            dlist.insert_after(a, 15)
            dlist.delete_tail()
        _validate(out.read_text(encoding="utf-8"))

    def test_reverse(self, tmp_path: Path) -> None:
        r"""测试反转"""
        out = tmp_path / "dl.toml"
        with double_linked_list(output=str(out)) as dlist:
            dlist.insert_tail(1)
            dlist.insert_tail(2)
            dlist.insert_tail(3)
            dlist.reverse()
        _validate(out.read_text(encoding="utf-8"))

    def test_delete_errors(self, tmp_path: Path) -> None:
        r"""测试删除错误"""
        out = tmp_path / "dl.toml"
        with pytest.raises(StructureError):
            with double_linked_list(output=str(out)) as dlist:
                dlist.delete_head()
        assert "[error]" in out.read_text(encoding="utf-8")

    def test_delete_specific_node(self, tmp_path: Path) -> None:
        r"""测试删除中间节点"""
        out = tmp_path / "dl.toml"
        with double_linked_list(output=str(out)) as dlist:
            dlist.insert_tail(10)
            b: int = dlist.insert_tail(20)
            dlist.insert_tail(30)
            dlist.delete(b)
        _validate(out.read_text(encoding="utf-8"))

    def test_insert_before_nonexistent_error(self, tmp_path: Path) -> None:
        r"""测试在不存在的节点前插入"""
        out = tmp_path / "dl.toml"
        with pytest.raises(StructureError):
            with double_linked_list(output=str(out)) as dlist:
                dlist.insert_tail(10)
                dlist.insert_before(999, 20)
        assert "[error]" in out.read_text(encoding="utf-8")

    def test_alias_and_step(self, tmp_path: Path) -> None:
        r"""测试别名和 step"""
        out = tmp_path / "dl.toml"
        with double_linked_list(output=str(out)) as dlist:
            a: int = dlist.insert_head(10)
            b: int = dlist.insert_tail(20)
            dlist.alias(a, "head")
            dlist.alias(b, "tail")
            dlist.step(note="查看双链表", highlights=[
                ds4viz.node(a, "focus", level=3),
                ds4viz.node(b, "active")
            ])
        content: str = out.read_text(encoding="utf-8")
        _validate(content)
        assert 'alias = "head"' in content
        assert 'alias = "tail"' in content

    def test_delete_head_single_node(self, tmp_path: Path) -> None:
        r"""测试单节点时删除头"""
        out = tmp_path / "dl.toml"
        with double_linked_list(output=str(out)) as dlist:
            dlist.insert_head(10)
            dlist.delete_head()
        content: str = out.read_text(encoding="utf-8")
        _validate(content)
        assert "head = -1" in content
        assert "tail = -1" in content

    def test_delete_tail_error_empty(self, tmp_path: Path) -> None:
        r"""测试空链表删除尾"""
        out = tmp_path / "dl.toml"
        with pytest.raises(StructureError):
            with double_linked_list(output=str(out)) as dlist:
                dlist.delete_tail()
        content: str = out.read_text(encoding="utf-8")
        _validate(content)
        assert "[error]" in content

    def test_delete_nonexistent_node_error(self, tmp_path: Path) -> None:
        r"""测试删除不存在的节点"""
        out = tmp_path / "dl.toml"
        with pytest.raises(StructureError):
            with double_linked_list(output=str(out)) as dlist:
                dlist.insert_head(10)
                dlist.delete(999)
        assert "[error]" in out.read_text(encoding="utf-8")

    def test_insert_after_nonexistent_error(self, tmp_path: Path) -> None:
        r"""测试在不存在的节点后插入"""
        out = tmp_path / "dl.toml"
        with pytest.raises(StructureError):
            with double_linked_list(output=str(out)) as dlist:
                dlist.insert_head(10)
                dlist.insert_after(999, 20)
        assert "[error]" in out.read_text(encoding="utf-8")


class TestBinaryTreeIntegration:
    r"""二叉树集成测试"""

    def test_basic_operations(self, tmp_path: Path) -> None:
        r"""测试二叉树基本操作"""
        out = tmp_path / "bt.toml"
        with binary_tree(output=str(out)) as bt:
            root: int = bt.insert_root(10)
            bt.alias(root, "root")
            left: int = bt.insert_left(root, 5)
            bt.insert_right(root, 15)
            bt.insert_left(left, 3)
        content: str = out.read_text(encoding="utf-8")
        _validate(content)
        assert 'kind = "binary_tree"' in content
        assert 'alias = "root"' in content

    def test_delete_subtree(self, tmp_path: Path) -> None:
        r"""测试删除子树"""
        out = tmp_path / "bt.toml"
        with binary_tree(output=str(out)) as bt:
            root: int = bt.insert_root(10)
            left: int = bt.insert_left(root, 5)
            bt.insert_left(left, 3)
            bt.delete(left)
        _validate(out.read_text(encoding="utf-8"))

    def test_update_value(self, tmp_path: Path) -> None:
        r"""测试更新值"""
        out = tmp_path / "bt.toml"
        with binary_tree(output=str(out)) as bt:
            root: int = bt.insert_root(10)
            bt.update_value(root, 100)
        _validate(out.read_text(encoding="utf-8"))

    def test_duplicate_root_error(self, tmp_path: Path) -> None:
        r"""测试重复插入根节点"""
        out = tmp_path / "bt.toml"
        with pytest.raises(StructureError):
            with binary_tree(output=str(out)) as bt:
                bt.insert_root(10)
                bt.insert_root(20)
        assert "[error]" in out.read_text(encoding="utf-8")

    def test_step_traversal(self, tmp_path: Path) -> None:
        r"""测试使用 step 模拟遍历"""
        out = tmp_path / "bt.toml"
        with binary_tree(output=str(out)) as bt:
            root: int = bt.insert_root(10)
            left: int = bt.insert_left(root, 5)
            right: int = bt.insert_right(root, 15)
            with bt.phase("preorder"):
                bt.step(note="访问 10", highlights=[
                        ds4viz.node(root, "focus", level=3)])
                bt.step(note="访问 5", highlights=[
                    ds4viz.node(root, "visited"), ds4viz.node(
                        left, "focus", level=3)
                ])
                bt.step(note="访问 15", highlights=[
                    ds4viz.node(root, "visited"), ds4viz.node(left, "visited"),
                    ds4viz.node(right, "focus", level=3)
                ])
        content: str = out.read_text(encoding="utf-8")
        _validate(content)
        assert 'phase = "preorder"' in content
        assert content.count('op = "observe"') == 3

    def test_delete_root(self, tmp_path: Path) -> None:
        r"""测试删除根节点及其所有子树"""
        out = tmp_path / "bt.toml"
        with binary_tree(output=str(out)) as bt:
            root: int = bt.insert_root(10)
            bt.insert_left(root, 5)
            bt.insert_right(root, 15)
            bt.delete(root)
        content: str = out.read_text(encoding="utf-8")
        _validate(content)
        assert "root = -1" in content

    def test_delete_leaf(self, tmp_path: Path) -> None:
        r"""测试删除叶节点"""
        out = tmp_path / "bt.toml"
        with binary_tree(output=str(out)) as bt:
            root: int = bt.insert_root(10)
            left: int = bt.insert_left(root, 5)
            bt.insert_right(root, 15)
            bt.delete(left)
        _validate(out.read_text(encoding="utf-8"))

    def test_insert_left_occupied_error(self, tmp_path: Path) -> None:
        r"""测试左子节点已存在时插入报错"""
        out = tmp_path / "bt.toml"
        with pytest.raises(StructureError):
            with binary_tree(output=str(out)) as bt:
                root: int = bt.insert_root(10)
                bt.insert_left(root, 5)
                bt.insert_left(root, 3)
        assert "[error]" in out.read_text(encoding="utf-8")

    def test_insert_nonexistent_parent_error(self, tmp_path: Path) -> None:
        r"""测试父节点不存在时插入报错"""
        out = tmp_path / "bt.toml"
        with pytest.raises(StructureError):
            with binary_tree(output=str(out)) as bt:
                bt.insert_root(10)
                bt.insert_left(999, 5)
        content: str = out.read_text(encoding="utf-8")
        _validate(content)
        assert "[error]" in content

    def test_update_nonexistent_node_error(self, tmp_path: Path) -> None:
        r"""测试更新不存在的节点报错"""
        out = tmp_path / "bt.toml"
        with pytest.raises(StructureError):
            with binary_tree(output=str(out)) as bt:
                bt.insert_root(10)
                bt.update_value(999, 100)
        assert "[error]" in out.read_text(encoding="utf-8")

    def test_insert_right_occupied_error(self, tmp_path: Path) -> None:
        r"""测试右子节点已存在时插入报错"""
        out = tmp_path / "bt.toml"
        with pytest.raises(StructureError):
            with binary_tree(output=str(out)) as bt:
                root: int = bt.insert_root(10)
                bt.insert_right(root, 15)
                bt.insert_right(root, 20)
        assert "[error]" in out.read_text(encoding="utf-8")

    def test_delete_nonexistent_node_error(self, tmp_path: Path) -> None:
        r"""测试删除不存在的节点报错"""
        out = tmp_path / "bt.toml"
        with pytest.raises(StructureError):
            with binary_tree(output=str(out)) as bt:
                bt.insert_root(10)
                bt.delete(999)
        assert "[error]" in out.read_text(encoding="utf-8")

    def test_string_values(self, tmp_path: Path) -> None:
        r"""测试字符串值节点"""
        out = tmp_path / "bt.toml"
        with binary_tree(output=str(out)) as bt:
            root: int = bt.insert_root("A")
            bt.insert_left(root, "B")
            bt.insert_right(root, "C")
        _validate(out.read_text(encoding="utf-8"))


class TestBinarySearchTreeIntegration:
    r"""二叉搜索树集成测试"""

    def test_basic_operations(self, tmp_path: Path) -> None:
        r"""测试 BST 基本操作"""
        out = tmp_path / "bst.toml"
        with binary_search_tree(output=str(out)) as bst:
            bst.insert(10)
            bst.insert(5)
            bst.insert(15)
            bst.insert(3)
            bst.insert(7)
        content: str = out.read_text(encoding="utf-8")
        _validate(content)
        assert 'kind = "bst"' in content

    def test_delete_leaf(self, tmp_path: Path) -> None:
        r"""测试删除叶节点"""
        out = tmp_path / "bst.toml"
        with binary_search_tree(output=str(out)) as bst:
            bst.insert(10)
            bst.insert(5)
            bst.insert(15)
            bst.delete(5)
        _validate(out.read_text(encoding="utf-8"))

    def test_delete_with_two_children(self, tmp_path: Path) -> None:
        r"""测试删除有两个子节点的节点"""
        out = tmp_path / "bst.toml"
        with binary_search_tree(output=str(out)) as bst:
            bst.insert(10)
            bst.insert(5)
            bst.insert(15)
            bst.insert(3)
            bst.insert(7)
            bst.delete(5)
        _validate(out.read_text(encoding="utf-8"))

    def test_delete_nonexistent_error(self, tmp_path: Path) -> None:
        r"""测试删除不存在的值"""
        out = tmp_path / "bst.toml"
        with pytest.raises(StructureError):
            with binary_search_tree(output=str(out)) as bst:
                bst.insert(10)
                bst.delete(999)
        assert "[error]" in out.read_text(encoding="utf-8")

    def test_delete_root(self, tmp_path: Path) -> None:
        r"""测试删除根节点"""
        out = tmp_path / "bst.toml"
        with binary_search_tree(output=str(out)) as bst:
            bst.insert(10)
            bst.insert(5)
            bst.insert(15)
            bst.delete(10)
        _validate(out.read_text(encoding="utf-8"))

    def test_delete_node_with_one_child(self, tmp_path: Path) -> None:
        r"""测试删除只有一个子节点的节点"""
        out = tmp_path / "bst.toml"
        with binary_search_tree(output=str(out)) as bst:
            bst.insert(10)
            bst.insert(5)
            bst.insert(3)
            bst.delete(5)
        _validate(out.read_text(encoding="utf-8"))

    def test_phase_and_alias(self, tmp_path: Path) -> None:
        r"""测试 BST phase 与 alias"""
        out = tmp_path / "bst.toml"
        with binary_search_tree(output=str(out)) as bst:
            with bst.phase("插入"):
                n10: int = bst.insert(10)
                bst.alias(n10, "root")
                bst.insert(5)
                bst.insert(15)
            with bst.phase("查找"):
                bst.step(note="查找 5", highlights=[
                    ds4viz.node(n10, "comparing", level=3)
                ])
        content: str = out.read_text(encoding="utf-8")
        _validate(content)
        assert 'phase = "插入"' in content
        assert 'phase = "查找"' in content
        assert 'alias = "root"' in content

    def test_sequential_insert_degenerate(self, tmp_path: Path) -> None:
        r"""测试顺序插入产生退化树"""
        out = tmp_path / "bst.toml"
        with binary_search_tree(output=str(out)) as bst:
            for v in [1, 2, 3, 4, 5]:
                bst.insert(v)
        _validate(out.read_text(encoding="utf-8"))

    def test_delete_all_nodes(self, tmp_path: Path) -> None:
        r"""测试依次删除所有节点"""
        out = tmp_path / "bst.toml"
        with binary_search_tree(output=str(out)) as bst:
            bst.insert(10)
            bst.insert(5)
            bst.insert(15)
            bst.delete(5)
            bst.delete(15)
            bst.delete(10)
        content: str = out.read_text(encoding="utf-8")
        _validate(content)
        assert "root = -1" in content

    def test_insert_descending_order(self, tmp_path: Path) -> None:
        r"""测试降序插入"""
        out = tmp_path / "bst.toml"
        with binary_search_tree(output=str(out)) as bst:
            for v in [5, 4, 3, 2, 1]:
                bst.insert(v)
        _validate(out.read_text(encoding="utf-8"))


class TestGraphUndirectedIntegration:
    r"""无向图集成测试"""

    def test_basic(self, tmp_path: Path) -> None:
        r"""测试无向图基本操作"""
        out = tmp_path / "gu.toml"
        with graph_undirected(output=str(out)) as g:
            g.add_node(0, "A")
            g.add_node(1, "B")
            g.add_edge(0, 1)
        content: str = out.read_text(encoding="utf-8")
        _validate(content)
        assert 'kind = "graph_undirected"' in content

    def test_edge_normalization(self, tmp_path: Path) -> None:
        r"""测试边规范化"""
        out = tmp_path / "gu.toml"
        with graph_undirected(output=str(out)) as g:
            g.add_node(0, "A")
            g.add_node(1, "B")
            g.add_edge(1, 0)
        content: str = out.read_text(encoding="utf-8")
        assert "from = 0" in content
        assert "to = 1" in content

    def test_self_loop_error(self, tmp_path: Path) -> None:
        r"""测试自环错误"""
        out = tmp_path / "gu.toml"
        with pytest.raises(StructureError):
            with graph_undirected(output=str(out)) as g:
                g.add_node(0, "A")
                g.add_edge(0, 0)

    def test_duplicate_edge_error(self, tmp_path: Path) -> None:
        r"""测试重复边错误"""
        out = tmp_path / "gu.toml"
        with pytest.raises(StructureError):
            with graph_undirected(output=str(out)) as g:
                g.add_node(0, "A")
                g.add_node(1, "B")
                g.add_edge(0, 1)
                g.add_edge(0, 1)

    def test_remove_node_and_edges(self, tmp_path: Path) -> None:
        r"""测试删除节点同时删除边"""
        out = tmp_path / "gu.toml"
        with graph_undirected(output=str(out)) as g:
            g.add_node(0, "A")
            g.add_node(1, "B")
            g.add_node(2, "C")
            g.add_edge(0, 1)
            g.add_edge(1, 2)
            g.remove_node(1)
        _validate(out.read_text(encoding="utf-8"))

    def test_step_with_edge_highlight(self, tmp_path: Path) -> None:
        r"""测试边高亮"""
        out = tmp_path / "gu.toml"
        with graph_undirected(output=str(out)) as g:
            g.add_node(0, "A")
            g.add_node(1, "B")
            g.add_edge(0, 1)
            g.step(highlights=[
                ds4viz.node(0, "visited"),
                ds4viz.node(1, "focus", level=3),
                ds4viz.edge(0, 1, "active")
            ])
        content: str = out.read_text(encoding="utf-8")
        _validate(content)
        assert 'kind = "edge"' in content

    def test_remove_edge(self, tmp_path: Path) -> None:
        r"""测试删除边"""
        out = tmp_path / "gu.toml"
        with graph_undirected(output=str(out)) as g:
            g.add_node(0, "A")
            g.add_node(1, "B")
            g.add_node(2, "C")
            g.add_edge(0, 1)
            g.add_edge(0, 2)
            g.remove_edge(0, 1)
        _validate(out.read_text(encoding="utf-8"))

    def test_update_node_label(self, tmp_path: Path) -> None:
        r"""测试更新节点标签"""
        out = tmp_path / "gu.toml"
        with graph_undirected(output=str(out)) as g:
            g.add_node(0, "A")
            g.update_node_label(0, "A-updated")
        content: str = out.read_text(encoding="utf-8")
        _validate(content)
        assert 'label = "A-updated"' in content

    def test_remove_nonexistent_node_error(self, tmp_path: Path) -> None:
        r"""测试删除不存在的节点"""
        out = tmp_path / "gu.toml"
        with pytest.raises(StructureError):
            with graph_undirected(output=str(out)) as g:
                g.add_node(0, "A")
                g.remove_node(999)

    def test_add_duplicate_node_error(self, tmp_path: Path) -> None:
        r"""测试添加重复节点"""
        out = tmp_path / "gu.toml"
        with pytest.raises(StructureError):
            with graph_undirected(output=str(out)) as g:
                g.add_node(0, "A")
                g.add_node(0, "B")

    def test_add_edge_nonexistent_node_error(self, tmp_path: Path) -> None:
        r"""测试在不存在的节点间添加边"""
        out = tmp_path / "gu.toml"
        with pytest.raises(StructureError):
            with graph_undirected(output=str(out)) as g:
                g.add_node(0, "A")
                g.add_edge(0, 1)

    def test_remove_nonexistent_edge_error(self, tmp_path: Path) -> None:
        r"""测试删除不存在的边"""
        out = tmp_path / "gu.toml"
        with pytest.raises(StructureError):
            with graph_undirected(output=str(out)) as g:
                g.add_node(0, "A")
                g.add_node(1, "B")
                g.remove_edge(0, 1)

    def test_duplicate_edge_reversed_error(self, tmp_path: Path) -> None:
        r"""测试反向添加重复边"""
        out = tmp_path / "gu.toml"
        with pytest.raises(StructureError):
            with graph_undirected(output=str(out)) as g:
                g.add_node(0, "A")
                g.add_node(1, "B")
                g.add_edge(0, 1)
                g.add_edge(1, 0)


class TestGraphDirectedIntegration:
    r"""有向图集成测试"""

    def test_basic(self, tmp_path: Path) -> None:
        r"""测试有向图基本操作"""
        out = tmp_path / "gd.toml"
        with graph_directed(output=str(out)) as g:
            g.add_node(0, "A")
            g.add_node(1, "B")
            g.add_edge(0, 1)
            g.add_edge(1, 0)
        content: str = out.read_text(encoding="utf-8")
        _validate(content)
        assert 'kind = "graph_directed"' in content

    def test_no_normalization(self, tmp_path: Path) -> None:
        r"""测试有向图不规范化"""
        out = tmp_path / "gd.toml"
        with graph_directed(output=str(out)) as g:
            g.add_node(0, "A")
            g.add_node(1, "B")
            g.add_edge(1, 0)
        content: str = out.read_text(encoding="utf-8")
        assert "from = 1" in content
        assert "to = 0" in content

    def test_self_loop_error(self, tmp_path: Path) -> None:
        r"""测试自环错误"""
        out = tmp_path / "gd.toml"
        with pytest.raises(StructureError):
            with graph_directed(output=str(out)) as g:
                g.add_node(0, "A")
                g.add_edge(0, 0)

    def test_remove_node_and_edges(self, tmp_path: Path) -> None:
        r"""测试删除节点同时删除关联入边与出边"""
        out = tmp_path / "gd.toml"
        with graph_directed(output=str(out)) as g:
            g.add_node(0, "A")
            g.add_node(1, "B")
            g.add_node(2, "C")
            g.add_edge(0, 1)
            g.add_edge(1, 2)
            g.add_edge(2, 1)
            g.remove_node(1)
        _validate(out.read_text(encoding="utf-8"))

    def test_remove_edge(self, tmp_path: Path) -> None:
        r"""测试删除有向边"""
        out = tmp_path / "gd.toml"
        with graph_directed(output=str(out)) as g:
            g.add_node(0, "A")
            g.add_node(1, "B")
            g.add_edge(0, 1)
            g.add_edge(1, 0)
            g.remove_edge(0, 1)
        _validate(out.read_text(encoding="utf-8"))

    def test_duplicate_edge_error(self, tmp_path: Path) -> None:
        r"""测试重复有向边错误"""
        out = tmp_path / "gd.toml"
        with pytest.raises(StructureError):
            with graph_directed(output=str(out)) as g:
                g.add_node(0, "A")
                g.add_node(1, "B")
                g.add_edge(0, 1)
                g.add_edge(0, 1)

    def test_add_duplicate_node_error(self, tmp_path: Path) -> None:
        r"""测试添加重复节点"""
        out = tmp_path / "gd.toml"
        with pytest.raises(StructureError):
            with graph_directed(output=str(out)) as g:
                g.add_node(0, "A")
                g.add_node(0, "A")

    def test_update_node_label(self, tmp_path: Path) -> None:
        r"""测试更新有向图节点标签"""
        out = tmp_path / "gd.toml"
        with graph_directed(output=str(out)) as g:
            g.add_node(0, "甲")
            g.update_node_label(0, "甲-修改")
        content: str = out.read_text(encoding="utf-8")
        _validate(content)
        assert 'label = "甲-修改"' in content

    def test_add_edge_nonexistent_node_error(self, tmp_path: Path) -> None:
        r"""测试在不存在的节点间添加有向边"""
        out = tmp_path / "gd.toml"
        with pytest.raises(StructureError):
            with graph_directed(output=str(out)) as g:
                g.add_node(0, "A")
                g.add_edge(0, 1)

    def test_remove_nonexistent_edge_error(self, tmp_path: Path) -> None:
        r"""测试删除不存在的有向边"""
        out = tmp_path / "gd.toml"
        with pytest.raises(StructureError):
            with graph_directed(output=str(out)) as g:
                g.add_node(0, "A")
                g.add_node(1, "B")
                g.remove_edge(0, 1)


class TestGraphWeightedIntegration:
    r"""带权图集成测试"""

    def test_basic(self, tmp_path: Path) -> None:
        r"""测试带权图基本操作"""
        out = tmp_path / "gw.toml"
        with graph_weighted(output=str(out)) as g:
            g.add_node(0, "A")
            g.add_node(1, "B")
            g.add_edge(0, 1, 3.5)
        content: str = out.read_text(encoding="utf-8")
        _validate(content)
        assert 'kind = "graph_weighted"' in content
        assert "weight = 3.5" in content

    def test_update_weight(self, tmp_path: Path) -> None:
        r"""测试更新权重"""
        out = tmp_path / "gw.toml"
        with graph_weighted(output=str(out)) as g:
            g.add_node(0, "A")
            g.add_node(1, "B")
            g.add_edge(0, 1, 1.0)
            g.update_weight(0, 1, 5.0)
        _validate(out.read_text(encoding="utf-8"))

    def test_alias_and_highlights(self, tmp_path: Path) -> None:
        r"""测试别名和高亮"""
        out = tmp_path / "gw.toml"
        with graph_weighted(output=str(out)) as g:
            g.add_node(0, "起点")
            g.alias(0, "source")
            g.add_node(1, "终点")
            g.add_edge(0, 1, 2.0)
            g.amend(highlights=[
                ds4viz.node(0, "active"),
                ds4viz.node(1, "active"),
                ds4viz.edge(0, 1, "focus", level=3)
            ])
        content: str = out.read_text(encoding="utf-8")
        _validate(content)
        assert 'alias = "source"' in content
        assert "highlights" in content

    def test_remove_node(self, tmp_path: Path) -> None:
        r"""测试删除节点"""
        out = tmp_path / "gw.toml"
        with graph_weighted(output=str(out)) as g:
            g.add_node(0, "A")
            g.add_node(1, "B")
            g.add_edge(0, 1, 1.0)
            g.remove_node(1)
        _validate(out.read_text(encoding="utf-8"))

    def test_update_nonexistent_edge_error(self, tmp_path: Path) -> None:
        r"""测试更新不存在的边"""
        out = tmp_path / "gw.toml"
        with pytest.raises(StructureError):
            with graph_weighted(output=str(out)) as g:
                g.add_node(0, "A")
                g.add_node(1, "B")
                g.update_weight(0, 1, 5.0)

    def test_negative_weight(self, tmp_path: Path) -> None:
        r"""测试负权重"""
        out = tmp_path / "gw.toml"
        with graph_weighted(output=str(out)) as g:
            g.add_node(0, "A")
            g.add_node(1, "B")
            g.add_edge(0, 1, -5.5)
        _validate(out.read_text(encoding="utf-8"))

    def test_self_loop_error(self, tmp_path: Path) -> None:
        r"""测试自环错误"""
        out = tmp_path / "gw.toml"
        with pytest.raises(StructureError):
            with graph_weighted(output=str(out)) as g:
                g.add_node(0, "A")
                g.add_edge(0, 0, 1.0)

    def test_duplicate_edge_error(self, tmp_path: Path) -> None:
        r"""测试重复边错误"""
        out = tmp_path / "gw.toml"
        with pytest.raises(StructureError):
            with graph_weighted(output=str(out)) as g:
                g.add_node(0, "A")
                g.add_node(1, "B")
                g.add_edge(0, 1, 1.0)
                g.add_edge(0, 1, 2.0)

    def test_add_duplicate_node_error(self, tmp_path: Path) -> None:
        r"""测试添加重复节点"""
        out = tmp_path / "gw.toml"
        with pytest.raises(StructureError):
            with graph_weighted(output=str(out)) as g:
                g.add_node(0, "A")
                g.add_node(0, "B")

    def test_update_node_label(self, tmp_path: Path) -> None:
        r"""测试更新节点标签"""
        out = tmp_path / "gw.toml"
        with graph_weighted(output=str(out)) as g:
            g.add_node(0, "Old")
            g.update_node_label(0, "New")
        content: str = out.read_text(encoding="utf-8")
        _validate(content)
        assert 'label = "New"' in content

    def test_integer_weight(self, tmp_path: Path) -> None:
        r"""测试整数权重"""
        out = tmp_path / "gw.toml"
        with graph_weighted(output=str(out)) as g:
            g.add_node(0, "A")
            g.add_node(1, "B")
            g.add_edge(0, 1, 7)
        _validate(out.read_text(encoding="utf-8"))

    def test_remove_edge(self, tmp_path: Path) -> None:
        r"""测试删除带权边"""
        out = tmp_path / "gw.toml"
        with graph_weighted(output=str(out)) as g:
            g.add_node(0, "A")
            g.add_node(1, "B")
            g.add_node(2, "C")
            g.add_edge(0, 1, 1.0)
            g.add_edge(1, 2, 2.0)
            g.remove_edge(0, 1)
        _validate(out.read_text(encoding="utf-8"))

    def test_add_edge_nonexistent_node_error(self, tmp_path: Path) -> None:
        r"""测试在不存在的节点间添加带权边"""
        out = tmp_path / "gw.toml"
        with pytest.raises(StructureError):
            with graph_weighted(output=str(out)) as g:
                g.add_node(0, "A")
                g.add_edge(0, 1, 1.0)

    def test_remove_nonexistent_edge_error(self, tmp_path: Path) -> None:
        r"""测试删除不存在的带权边"""
        out = tmp_path / "gw.toml"
        with pytest.raises(StructureError):
            with graph_weighted(output=str(out)) as g:
                g.add_node(0, "A")
                g.add_node(1, "B")
                g.remove_edge(0, 1)

    def test_zero_weight(self, tmp_path: Path) -> None:
        r"""测试零权重"""
        out = tmp_path / "gw.toml"
        with graph_weighted(output=str(out)) as g:
            g.add_node(0, "A")
            g.add_node(1, "B")
            g.add_edge(0, 1, 0)
        _validate(out.read_text(encoding="utf-8"))


class TestGlobalConfigIntegration:
    r"""全局配置集成测试"""

    def test_config_remarks(self, tmp_path: Path) -> None:
        r"""测试配置 remarks"""
        out = tmp_path / "cfg.toml"
        config(output_path=str(out), title="Title",
               author="Author", comment="Comment")
        with stack() as s:
            s.push(1)
        content: str = out.read_text(encoding="utf-8")
        assert 'title = "Title"' in content
        assert 'author = "Author"' in content
        assert 'comment = "Comment"' in content
        config()

    def test_config_output_path(self, tmp_path: Path) -> None:
        r"""测试配置输出路径"""
        out = tmp_path / "custom.toml"
        config(output_path=str(out))
        with stack() as s:
            s.push(1)
        assert out.exists()
        _validate(out.read_text(encoding="utf-8"))
        config()

    def test_partial_remarks_title_only(self, tmp_path: Path) -> None:
        r"""测试仅设置 title 的部分 remarks"""
        out = tmp_path / "cfg.toml"
        config(output_path=str(out), title="仅标题")
        with stack() as s:
            s.push(1)
        content: str = out.read_text(encoding="utf-8")
        _validate(content)
        assert 'title = "仅标题"' in content
        config()

    def test_structure_output_overrides_global(self, tmp_path: Path) -> None:
        r"""测试结构级 output 覆盖全局配置"""
        global_out = tmp_path / "global.toml"
        struct_out = tmp_path / "struct.toml"
        config(output_path=str(global_out))
        with stack(output=str(struct_out)) as s:
            s.push(1)
        assert struct_out.exists()
        _validate(struct_out.read_text(encoding="utf-8"))
        config()

    def test_no_remarks_when_empty(self, tmp_path: Path) -> None:
        r"""测试无 remarks 参数时输出有效"""
        out = tmp_path / "cfg.toml"
        config(output_path=str(out))
        with stack() as s:
            s.push(1)
        content: str = out.read_text(encoding="utf-8")
        _validate(content)
        config()

    def test_partial_remarks_author_only(self, tmp_path: Path) -> None:
        r"""测试仅设置 author 的部分 remarks"""
        out = tmp_path / "cfg.toml"
        config(output_path=str(out), author="TestAuthor")
        with stack() as s:
            s.push(1)
        content: str = out.read_text(encoding="utf-8")
        _validate(content)
        assert 'author = "TestAuthor"' in content
        config()

    def test_partial_remarks_comment_only(self, tmp_path: Path) -> None:
        r"""测试仅设置 comment 的部分 remarks"""
        out = tmp_path / "cfg.toml"
        config(output_path=str(out), comment="just a comment")
        with stack() as s:
            s.push(1)
        content: str = out.read_text(encoding="utf-8")
        _validate(content)
        assert 'comment = "just a comment"' in content
        config()


class TestEdgeCases:
    r"""边缘情况测试"""

    def test_empty_structure(self, tmp_path: Path) -> None:
        r"""测试空数据结构"""
        out = tmp_path / "empty.toml"
        with stack(output=str(out)):
            ...
        content: str = out.read_text(encoding="utf-8")
        _validate(content)
        assert content.count("[[states]]") == 1

    def test_special_characters_in_string(self, tmp_path: Path) -> None:
        r"""测试特殊字符"""
        out = tmp_path / "special.toml"
        with stack(output=str(out)) as s:
            s.push('hello"world')
            s.push("line1\\nline2")
        _validate(out.read_text(encoding="utf-8"))

    def test_many_operations(self, tmp_path: Path) -> None:
        r"""测试大量操作"""
        out = tmp_path / "many.toml"
        with stack(output=str(out)) as s:
            for i in range(100):
                s.push(i)
            for _ in range(50):
                s.pop()
        content: str = out.read_text(encoding="utf-8")
        _validate(content)
        assert content.count("[[states]]") == 151
        assert content.count("[[steps]]") == 150

    def test_label_boundary(self, tmp_path: Path) -> None:
        r"""测试标签边界"""
        out = tmp_path / "lbl.toml"
        with graph_undirected(output=str(out)) as g:
            g.add_node(0, "A")
            g.add_node(1, "A" * 32)
        _validate(out.read_text(encoding="utf-8"))

    def test_label_too_long_error(self, tmp_path: Path) -> None:
        r"""测试标签过长"""
        out = tmp_path / "lbl.toml"
        with pytest.raises(StructureError):
            with graph_undirected(output=str(out)) as g:
                g.add_node(0, "A" * 33)

    def test_label_empty_error(self, tmp_path: Path) -> None:
        r"""测试空标签"""
        out = tmp_path / "lbl.toml"
        with pytest.raises(StructureError):
            with graph_undirected(output=str(out)) as g:
                g.add_node(0, "")

    def test_unicode_strings(self, tmp_path: Path) -> None:
        r"""测试 Unicode 字符串"""
        out = tmp_path / "uni.toml"
        with stack(output=str(out)) as s:
            s.push("你好")
            s.push("🌍")
        _validate(out.read_text(encoding="utf-8"))

    def test_nested_phases_inner_priority(self, tmp_path: Path) -> None:
        r"""测试嵌套 phase 内层优先"""
        out = tmp_path / "np.toml"
        with queue(output=str(out)) as q:
            with q.phase("outer"):
                q.enqueue(1)
                with q.phase("inner"):
                    q.enqueue(2)
                q.enqueue(3)
        content: str = out.read_text(encoding="utf-8")
        _validate(content)
        assert 'phase = "inner"' in content
        assert 'phase = "outer"' in content

    def test_all_highlight_styles(self, tmp_path: Path) -> None:
        r"""测试所有预定义高亮样式"""
        out = tmp_path / "hl.toml"
        styles: list[str] = ["focus", "visited",
                             "active", "comparing", "found", "error"]
        with binary_tree(output=str(out)) as bt:
            root: int = bt.insert_root(10)
            for style in styles:
                bt.step(note=f"style={style}", highlights=[
                    ds4viz.node(root, style, level=1)
                ])
        content: str = out.read_text(encoding="utf-8")
        _validate(content)
        for style in styles:
            assert f'style = "{style}"' in content

    def test_float_precision_values(self, tmp_path: Path) -> None:
        r"""测试浮点数精度值"""
        out = tmp_path / "fp.toml"
        with stack(output=str(out)) as s:
            s.push(0.1)
            s.push(1e-10)
            s.push(1e10)
            s.push(-0.0)
        _validate(out.read_text(encoding="utf-8"))

    def test_boolean_values_in_queue(self, tmp_path: Path) -> None:
        r"""测试队列中的布尔值"""
        out = tmp_path / "bq.toml"
        with queue(output=str(out)) as q:
            q.enqueue(True)
            q.enqueue(False)
            val = q.dequeue()
            assert val is True
            val = q.dequeue()
            assert val is False
        _validate(out.read_text(encoding="utf-8"))

    def test_empty_graph(self, tmp_path: Path) -> None:
        r"""测试空图结构"""
        out = tmp_path / "eg.toml"
        with graph_directed(output=str(out)):
            ...
        content: str = out.read_text(encoding="utf-8")
        _validate(content)
        assert content.count("[[states]]") == 1

    def test_amend_overwrite(self, tmp_path: Path) -> None:
        r"""测试连续两次 amend 覆盖"""
        out = tmp_path / "ao.toml"
        with stack(output=str(out)) as s:
            s.push(10)
            s.amend(note="第一次")
            s.amend(note="第二次")
        content: str = out.read_text(encoding="utf-8")
        _validate(content)
        assert 'note = "第二次"' in content

    def test_step_only_no_mutations(self, tmp_path: Path) -> None:
        r"""测试仅使用 step 不做结构变更, step 不生成新的 state 快照, before == after"""
        out = tmp_path / "so.toml"
        with stack(output=str(out)) as s:
            s.step(note="观察空栈")
        content: str = out.read_text(encoding="utf-8")
        _validate(content)
        assert 'op = "observe"' in content
        assert content.count("[[states]]") == 1

    def test_step_line_offset_for_wrapped_visit(self, tmp_path: Path) -> None:
        r"""
        测试 step(line_offset=1) 在封装函数场景下记录外层调用行号
        """
        out: Path = tmp_path / "wrapped_step_line.toml"
        observe_lines: list[int] = []

        with binary_tree(output=str(out)) as tree:
            root: int = tree.insert_root(10)
            n5: int = tree.insert_left(root, 5)
            n15: int = tree.insert_right(root, 15)
            visited: list[int] = []

            def visit(node_id: int, label: int) -> None:
                r"""
                封装访问逻辑, 使用 line_offset=1 记录外层调用行

                :param node_id: 节点 ID
                :param label: 节点标签值
                :return None: 无返回值
                """
                tree.step(
                    note=f"访问 {label}",
                    highlights=[
                        *[ds4viz.node(v, "visited") for v in visited],
                        ds4viz.node(node_id, "focus", level=3)
                    ],
                    line_offset=1
                )
                visited.append(node_id)

            observe_lines.append(inspect.currentframe().f_lineno + 1)
            visit(root, 10)
            observe_lines.append(inspect.currentframe().f_lineno + 1)
            visit(n5, 5)
            observe_lines.append(inspect.currentframe().f_lineno + 1)
            visit(n15, 15)

        content: str = out.read_text(encoding="utf-8")
        _validate(content)
        data: dict[str, Any] = tomllib.loads(content)

        observe_steps: list[dict[str, Any]] = [
            s for s in data["steps"] if s["op"] == "observe"
        ]
        assert len(observe_steps) == 3

        actual_lines: list[int] = [s["code"]["line"] for s in observe_steps]
        assert actual_lines == observe_lines
        
    def test_bst_inorder_traversal_line_numbers(
        self, tmp_path: Path
    ) -> None:
        r"""测试二叉搜索树中序遍历中每个 observe 步骤的行号正确性,
        验证 before == after 约束, 状态不增长, 以及 commit.line
        指向最后一次操作的行号"""
        output: Path = tmp_path / "bst_traversal.toml"
        observe_lines: list[int] = []

        with ds4viz.structures.binary_search_tree(output=str(output)) as tree:
            tree.insert(4)
            tree.insert(2)
            tree.insert(6)
            tree.insert(1)
            tree.insert(3)
            # 模拟中序遍历: 1 → 2 → 3 → 4 → 6
            observe_lines.append(inspect.currentframe().f_lineno + 1)
            tree.step(note="遍历到节点 1")
            observe_lines.append(inspect.currentframe().f_lineno + 1)
            tree.step(note="遍历到节点 2")
            observe_lines.append(inspect.currentframe().f_lineno + 1)
            tree.step(note="遍历到节点 3")
            observe_lines.append(inspect.currentframe().f_lineno + 1)
            tree.step(note="遍历到节点 4")
            observe_lines.append(inspect.currentframe().f_lineno + 1)
            tree.step(note="遍历到节点 6")

        content: str = output.read_text(encoding="utf-8")
        _validate(content)
        data: dict = tomllib.loads(content)

        # ---- 验证 observe 步骤 ----
        observe_steps: list[dict] = [
            s for s in data["steps"] if s["op"] == "observe"
        ]
        assert len(observe_steps) == 5, (
            f"应有 5 个 observe 步骤, 实际 {len(observe_steps)} 个"
        )

        for i, obs in enumerate(observe_steps):
            # before == after
            assert obs["before"] == obs["after"], (
                f"observe[{i}]: before={obs['before']} "
                f"!= after={obs['after']}"
            )
            # code 字段存在
            assert obs.get("code") is not None, (
                f"observe[{i}]: 缺少 code 字段"
            )
            # 行号与源码一致
            actual_line: int = obs["code"]["line"]
            assert actual_line == observe_lines[i], (
                f"observe[{i}]: 期望行号 {observe_lines[i]}, "
                f"实际行号 {actual_line}"
            )

        # ---- 验证 mutate 步骤 (insert) 的 before != after ----
        mutate_steps: list[dict] = [
            s for s in data["steps"] if s["op"] != "observe"
        ]
        assert len(mutate_steps) == 5, (
            f"应有 5 个 insert 步骤, 实际 {len(mutate_steps)} 个"
        )
        for ms in mutate_steps:
            assert ms["before"] != ms["after"], (
                f"insert 步骤 before 应不等于 after, "
                f"got before={ms['before']}, after={ms['after']}"
            )

        # ---- 验证状态数: init(1) + 5 次 insert = 6, observe 不增长 ----
        assert len(data["states"]) == 6, (
            f"应有 6 个状态, 实际 {len(data['states'])} 个"
        )

        # ---- 验证 commit.line 指向最后一次操作的行号 ----
        commit_line: int = data["result"]["commit"]["line"]
        assert commit_line == observe_lines[-1], (
            f"commit.line 应为 {observe_lines[-1]}, "
            f"实际为 {commit_line}"
        )
        
    def test_highlight_level_boundaries(self, tmp_path: Path) -> None:
        r"""测试高亮 level 边界值 1 和 9"""
        out = tmp_path / "hl.toml"
        with binary_tree(output=str(out)) as bt:
            root: int = bt.insert_root(10)
            bt.step(highlights=[ds4viz.node(root, "focus", level=1)])
            bt.step(highlights=[ds4viz.node(root, "focus", level=9)])
        content: str = out.read_text(encoding="utf-8")
        _validate(content)
        assert "level = 1" in content
        assert "level = 9" in content
        
    def test_bst_inorder_traversal_line_numbers(
        self, tmp_path: Path
    ) -> None:
        r"""测试二叉搜索树中序遍历中每个 observe 步骤的行号正确性,
        验证 before == after 约束, 状态不增长, 以及 commit.line
        指向最后一次操作的行号"""
        output: Path = tmp_path / "bst_traversal.toml"
        observe_lines: list[int] = []

        with ds4viz.structures.BinarySearchTree(output=str(output)) as tree:
            tree.insert(4)
            tree.insert(2)
            tree.insert(6)
            tree.insert(1)
            tree.insert(3)
            # 模拟中序遍历: 1 → 2 → 3 → 4 → 6
            observe_lines.append(inspect.currentframe().f_lineno + 1)
            tree.step(note="遍历到节点 1")
            observe_lines.append(inspect.currentframe().f_lineno + 1)
            tree.step(note="遍历到节点 2")
            observe_lines.append(inspect.currentframe().f_lineno + 1)
            tree.step(note="遍历到节点 3")
            observe_lines.append(inspect.currentframe().f_lineno + 1)
            tree.step(note="遍历到节点 4")
            observe_lines.append(inspect.currentframe().f_lineno + 1)
            tree.step(note="遍历到节点 6")

        content: str = output.read_text(encoding="utf-8")
        _validate(content)
        data: dict = tomllib.loads(content)

        # ---- 验证 observe 步骤 ----
        observe_steps: list[dict] = [
            s for s in data["steps"] if s["op"] == "observe"
        ]
        assert len(observe_steps) == 5, (
            f"应有 5 个 observe 步骤, 实际 {len(observe_steps)} 个"
        )

        for i, obs in enumerate(observe_steps):
            # before == after
            assert obs["before"] == obs["after"], (
                f"observe[{i}]: before={obs['before']} "
                f"!= after={obs['after']}"
            )
            # code 字段存在
            assert obs.get("code") is not None, (
                f"observe[{i}]: 缺少 code 字段"
            )
            # 行号与源码一致
            actual_line: int = obs["code"]["line"]
            assert actual_line == observe_lines[i], (
                f"observe[{i}]: 期望行号 {observe_lines[i]}, "
                f"实际行号 {actual_line}"
            )

        # ---- 验证 mutate 步骤 (insert) 的 before != after ----
        mutate_steps: list[dict] = [
            s for s in data["steps"] if s["op"] != "observe"
        ]
        assert len(mutate_steps) == 5, (
            f"应有 5 个 insert 步骤, 实际 {len(mutate_steps)} 个"
        )
        for ms in mutate_steps:
            assert ms["before"] != ms["after"], (
                f"insert 步骤 before 应不等于 after, "
                f"got before={ms['before']}, after={ms['after']}"
            )

        # ---- 验证状态数: init(1) + 5 次 insert = 6, observe 不增长 ----
        assert len(data["states"]) == 6, (
            f"应有 6 个状态, 实际 {len(data['states'])} 个"
        )

        # ---- 验证 commit.line 指向最后一次操作的行号 ----
        commit_line: int = data["result"]["commit"]["line"]
        assert commit_line == observe_lines[-1], (
            f"commit.line 应为 {observe_lines[-1]}, "
            f"实际为 {commit_line}"
        )

    def test_empty_binary_tree(self, tmp_path: Path) -> None:
        r"""测试空二叉树"""
        out = tmp_path / "ebt.toml"
        with binary_tree(output=str(out)):
            ...
        content: str = out.read_text(encoding="utf-8")
        _validate(content)
        assert "root = -1" in content

    def test_empty_single_linked_list(self, tmp_path: Path) -> None:
        r"""测试空单链表"""
        out = tmp_path / "esl.toml"
        with single_linked_list(output=str(out)):
            ...
        content: str = out.read_text(encoding="utf-8")
        _validate(content)
        assert "head = -1" in content

    def test_empty_double_linked_list(self, tmp_path: Path) -> None:
        r"""测试空双向链表"""
        out = tmp_path / "edl.toml"
        with double_linked_list(output=str(out)):
            ...
        content: str = out.read_text(encoding="utf-8")
        _validate(content)
        assert "head = -1" in content
        assert "tail = -1" in content

    def test_amend_clears_highlights(self, tmp_path: Path) -> None:
        r"""测试 amend 传入空列表清除高亮"""
        out = tmp_path / "ac.toml"
        with stack(output=str(out)) as s:
            s.push(10)
            s.amend(highlights=[ds4viz.item(0, "focus", level=3)])
            s.amend(highlights=[])
        content: str = out.read_text(encoding="utf-8")
        _validate(content)


class TestComplexScenarios:
    r"""复杂场景测试"""

    def test_bst_complex_deletion(self, tmp_path: Path) -> None:
        r"""测试 BST 复杂删除"""
        out = tmp_path / "bst.toml"
        with binary_search_tree(output=str(out)) as bst:
            for v in [50, 30, 70, 20, 40, 60, 80, 35, 45]:
                bst.insert(v)
            bst.delete(30)
            bst.delete(50)
        _validate(out.read_text(encoding="utf-8"))

    def test_graph_complex(self, tmp_path: Path) -> None:
        r"""测试图复杂操作"""
        out = tmp_path / "g.toml"
        with graph_weighted(output=str(out)) as g:
            for i in range(5):
                g.add_node(i, chr(ord('A') + i))
            g.add_edge(0, 1, 1.0)
            g.add_edge(1, 2, 1.5)
            g.add_edge(2, 3, 2.5)
            g.add_edge(3, 4, 1.0)
            g.update_weight(0, 1, 10.0)
            g.remove_node(2)
            g.update_node_label(0, "Start")
        _validate(out.read_text(encoding="utf-8"))

    def test_dlist_complex(self, tmp_path: Path) -> None:
        r"""测试双向链表复杂操作"""
        out = tmp_path / "dl.toml"
        with double_linked_list(output=str(out)) as dlist:
            nodes: list[int] = []
            for i in range(5):
                nodes.append(dlist.insert_tail(i))
            dlist.insert_after(nodes[2], 100)
            dlist.insert_before(nodes[2], 200)
            dlist.delete_head()
            dlist.delete_tail()
            dlist.reverse()
        _validate(out.read_text(encoding="utf-8"))

    def test_all_structures_context_manager(self, tmp_path: Path) -> None:
        r"""测试所有数据结构都是上下文管理器"""
        factories = [
            stack, queue, single_linked_list, double_linked_list,
            binary_tree, binary_search_tree,
            graph_undirected, graph_directed, graph_weighted
        ]
        for factory in factories:
            out = tmp_path / f"{factory.__name__}.toml"
            with factory(output=str(out)):
                ...
            assert out.exists()
            _validate(out.read_text(encoding="utf-8"))

    def test_bfs_directed_graph_simulation(self, tmp_path: Path) -> None:
        r"""测试有向图 BFS 模拟"""
        out = tmp_path / "bfs.toml"
        with graph_directed(output=str(out)) as g:
            with g.phase("构建"):
                for i in range(4):
                    g.add_node(i, chr(ord('A') + i))
                g.add_edge(0, 1)
                g.add_edge(0, 2)
                g.add_edge(1, 3)
                g.add_edge(2, 3)
            visited: list[int] = []
            with g.phase("BFS"):
                visited.append(0)
                g.step(note="起点 A", highlights=[
                    ds4viz.node(0, "focus", level=3)
                ])
                g.step(note="访问 B, C", highlights=[
                    ds4viz.node(0, "visited"),
                    ds4viz.node(1, "focus", level=2),
                    ds4viz.node(2, "focus", level=2)
                ])
                visited.extend([1, 2])
                g.step(note="访问 D", highlights=[
                    *[ds4viz.node(v, "visited") for v in visited],
                    ds4viz.node(3, "focus", level=2)
                ])
                visited.append(3)
                g.step(note="BFS 完成", highlights=[
                    *[ds4viz.node(v, "visited") for v in visited]
                ])
        content: str = out.read_text(encoding="utf-8")
        _validate(content)
        assert 'phase = "BFS"' in content

    def test_linked_list_cycle_operations(self, tmp_path: Path) -> None:
        r"""测试链表循环操作: 构建, 删除, 重建"""
        out = tmp_path / "lc.toml"
        with single_linked_list(output=str(out)) as slist:
            with slist.phase("第一轮"):
                a: int = slist.insert_head(1)
                b: int = slist.insert_tail(2)
                c: int = slist.insert_tail(3)
                slist.delete(b)
                slist.delete(c)
                slist.delete(a)
            with slist.phase("第二轮"):
                d: int = slist.insert_head(10)
                e: int = slist.insert_tail(20)
                slist.step(note="新链表", highlights=[
                    ds4viz.node(d, "focus", level=3),
                    ds4viz.node(e, "visited")
                ])
        content: str = out.read_text(encoding="utf-8")
        _validate(content)
        assert 'phase = "第一轮"' in content
        assert 'phase = "第二轮"' in content

    def test_deep_binary_tree(self, tmp_path: Path) -> None:
        r"""测试深层二叉树 (10 层, 全左子树)"""
        out = tmp_path / "deep.toml"
        with binary_tree(output=str(out)) as bt:
            parent: int = bt.insert_root(0)
            for i in range(1, 10):
                parent = bt.insert_left(parent, i)
        _validate(out.read_text(encoding="utf-8"))

    def test_complete_undirected_graph(self, tmp_path: Path) -> None:
        r"""测试完全无向图 (K5)"""
        out = tmp_path / "k5.toml"
        n: int = 5
        with graph_undirected(output=str(out)) as g:
            for i in range(n):
                g.add_node(i, chr(ord('A') + i))
            for i in range(n):
                for j in range(i + 1, n):
                    g.add_edge(i, j)
        _validate(out.read_text(encoding="utf-8"))

    def test_dijkstra_simulation(self, tmp_path: Path) -> None:
        r"""测试 Dijkstra 松弛过程模拟"""
        out = tmp_path / "dj.toml"
        with graph_weighted(output=str(out)) as g:
            with g.phase("构建"):
                g.add_node(0, "起点")
                g.alias(0, "source")
                g.add_node(1, "甲")
                g.add_node(2, "乙")
                g.add_node(3, "终点")
                g.add_edge(0, 1, 1)
                g.add_edge(0, 2, 4)
                g.add_edge(1, 2, 2)
                g.add_edge(2, 3, 3)
            with g.phase("松弛"):
                g.step(note="从起点松弛", highlights=[
                    ds4viz.node(0, "visited"),
                    ds4viz.node(1, "focus", level=2),
                    ds4viz.edge(0, 1, "active", level=3)
                ])
                g.step(note="从甲松弛", highlights=[
                    ds4viz.node(0, "visited"),
                    ds4viz.node(1, "visited"),
                    ds4viz.node(2, "focus", level=2),
                    ds4viz.edge(1, 2, "active", level=3)
                ])
                g.step(note="完成", highlights=[
                    ds4viz.node(0, "found"),
                    ds4viz.node(1, "found"),
                    ds4viz.node(2, "found"),
                    ds4viz.node(3, "found"),
                    ds4viz.edge(0, 1, "found", level=3),
                    ds4viz.edge(1, 2, "found", level=3),
                    ds4viz.edge(2, 3, "found", level=3)
                ])
        content: str = out.read_text(encoding="utf-8")
        _validate(content)
        assert 'alias = "source"' in content
        assert 'phase = "松弛"' in content

    def test_binary_tree_build_and_traverse(self, tmp_path: Path) -> None:
        r"""测试二叉树构建后完整前序遍历"""
        out = tmp_path / "btt.toml"
        with binary_tree(output=str(out)) as bt:
            with bt.phase("构建"):
                root: int = bt.insert_root(1)
                n2: int = bt.insert_left(root, 2)
                n3: int = bt.insert_right(root, 3)
                n4: int = bt.insert_left(n2, 4)
                n5: int = bt.insert_right(n2, 5)
            visited: list[int] = []
            with bt.phase("前序"):
                for nid, label in [(root, 1), (n2, 2), (n4, 4), (n5, 5), (n3, 3)]:
                    bt.step(
                        note=f"访问 {label}",
                        highlights=[
                            *[ds4viz.node(v, "visited") for v in visited],
                            ds4viz.node(nid, "focus", level=3)
                        ]
                    )
                    visited.append(nid)
        content: str = out.read_text(encoding="utf-8")
        _validate(content)
        assert content.count('op = "observe"') == 5

    def test_graph_build_remove_rebuild(self, tmp_path: Path) -> None:
        r"""测试图构建-删除-重建"""
        out = tmp_path / "grr.toml"
        with graph_directed(output=str(out)) as g:
            with g.phase("构建"):
                g.add_node(0, "A")
                g.add_node(1, "B")
                g.add_node(2, "C")
                g.add_edge(0, 1)
                g.add_edge(1, 2)
            with g.phase("清理"):
                g.remove_node(1)
            with g.phase("重建"):
                g.add_node(3, "D")
                g.add_edge(0, 3)
                g.add_edge(3, 2)
        content: str = out.read_text(encoding="utf-8")
        _validate(content)
        assert 'phase = "构建"' in content
        assert 'phase = "清理"' in content
        assert 'phase = "重建"' in content

    def test_slist_traversal_with_amend(self, tmp_path: Path) -> None:
        r"""测试单链表遍历中使用 amend"""
        out = tmp_path / "sla.toml"
        with single_linked_list(output=str(out)) as slist:
            a: int = slist.insert_tail(10)
            b: int = slist.insert_tail(20)
            c: int = slist.insert_tail(30)
            slist.step(note="开始遍历")
            slist.amend(highlights=[ds4viz.node(a, "focus", level=3)])
            slist.step(note="下一个节点", highlights=[
                ds4viz.node(a, "visited"),
                ds4viz.node(b, "focus", level=3)
            ])
            slist.step(note="最后一个节点", highlights=[
                ds4viz.node(a, "visited"),
                ds4viz.node(b, "visited"),
                ds4viz.node(c, "focus", level=3)
            ])
        content: str = out.read_text(encoding="utf-8")
        _validate(content)
        assert content.count('op = "observe"') == 3
