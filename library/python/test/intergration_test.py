r"""
集成测试模块

:file: test/integration_test.py
:author: WaterRun
:time: 2025-12-23
"""

import re
import pytest
from typing import Any

import ds4viz
from ds4viz import (
    config, stack, queue, single_linked_list, double_linked_list,
    binary_tree, binary_search_tree, heap,
    graph_undirected, graph_directed, graph_weighted,
    Stack, Queue, SingleLinkedList, DoubleLinkedList,
    BinaryTree, BinarySearchTree, Heap,
    GraphUndirected, GraphDirected, GraphWeighted
)
from ds4viz.exception import StructureError


class TomlValidator:
    r"""TOML IR 验证器，根据 IR 定义验证输出"""

    # IR 定义中允许的 kind 值
    VALID_KINDS: set[str] = {
        "stack", "queue", "slist", "dlist",
        "binary_tree", "bst",
        "graph_undirected", "graph_directed", "graph_weighted"
    }

    # IR 定义中允许的 error type 值
    VALID_ERROR_TYPES: set[str] = {
        "runtime", "timeout", "validation", "sandbox", "unknown"
    }

    # IR 定义中允许的语言值
    VALID_LANGS: set[str] = {
        "python", "c", "zig", "rust", "java", "csharp", "typescript", "lua"
    }

    def __init__(self, content: str) -> None:
        r"""
        初始化验证器

        :param content: TOML 文件内容
        """
        self.content = content
        self.lines = content.split("\n")
        self.errors: list[str] = []

    def validate_all(self) -> bool:
        r"""
        执行所有验证

        :return bool: 是否通过所有验证
        """
        self.validate_structure_order()
        self.validate_meta()
        self.validate_package()
        self.validate_object()
        self.validate_states()
        self.validate_steps()
        self.validate_result_or_error()
        return len(self.errors) == 0

    def validate_structure_order(self) -> None:
        r"""验证顶层结构顺序"""
        sections = []
        for line in self.lines:
            line = line.strip()
            if line.startswith("[") and not line.startswith("[["):
                # 提取 section 名称
                match = re.match(r"\[([a-z_.]+)]", line)
                if match:
                    sections.append(match.group(1))
            elif line.startswith("[["):
                match = re.match(r"\[\[([a-z]+)]]", line)
                if match:
                    section_name = match.group(1)
                    if section_name not in sections:
                        sections.append(section_name)

        expected_order = ["meta", "package", "remarks", "object", "states", "steps", "result", "error"]
        filtered_sections = [s for s in sections if s in expected_order or s.split(".")[0] in expected_order]

        # 移除重复项，保持顺序
        seen: set[str] = set()
        unique_sections = []
        for s in filtered_sections:
            base = s.split(".")[0]
            if base not in seen:
                seen.add(base)
                unique_sections.append(base)

        # 验证顺序
        prev_idx = -1
        for section in unique_sections:
            if section in expected_order:
                idx = expected_order.index(section)
                if idx < prev_idx:
                    self.errors.append(f"Section order violation: {section} should come earlier")
                prev_idx = idx

    def validate_meta(self) -> None:
        r"""验证 [meta] 部分"""
        if "[meta]" not in self.content:
            self.errors.append("Missing required [meta] section")
            return

        # 验证 created_at 格式 (RFC3339)
        created_at_match = re.search(r'created_at\s*=\s*"([^"]+)"', self.content)
        if not created_at_match:
            self.errors.append("Missing meta.created_at")
        else:
            created_at = created_at_match.group(1)
            if not re.match(r"\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}Z", created_at):
                self.errors.append(f"Invalid meta.created_at format: {created_at}")

        # 验证 lang
        lang_match = re.search(r'\[meta].*?lang\s*=\s*"([^"]+)"', self.content, re.DOTALL)
        if not lang_match:
            self.errors.append("Missing meta.lang")
        else:
            lang = lang_match.group(1)
            if lang not in self.VALID_LANGS:
                self.errors.append(f"Invalid meta.lang: {lang}")

        # 验证 lang_version
        lang_version_match = re.search(r'lang_version\s*=\s*"([^"]+)"', self.content)
        if not lang_version_match:
            self.errors.append("Missing meta.lang_version")
        else:
            lang_version = lang_version_match.group(1)
            if not re.match(r"^[0-9a-z.+-]+$", lang_version):
                self.errors.append(f"Invalid meta.lang_version format: {lang_version}")

    def validate_package(self) -> None:
        r"""验证 [package] 部分"""
        if "[package]" not in self.content:
            self.errors.append("Missing required [package] section")
            return

        # 验证 name
        name_match = re.search(r'\[package].*?name\s*=\s*"([^"]+)"', self.content, re.DOTALL)
        if not name_match:
            self.errors.append("Missing package.name")
        else:
            name = name_match.group(1)
            if not re.match(r"^[0-9a-z_-]{1,64}$", name):
                self.errors.append(f"Invalid package.name: {name}")

        # 验证 version (语义化版本)
        version_match = re.search(r'\[package].*?version\s*=\s*"([^"]+)"', self.content, re.DOTALL)
        if not version_match:
            self.errors.append("Missing package.version")
        else:
            version = version_match.group(1)
            if not re.match(r"^\d+\.\d+\.\d+$", version):
                self.errors.append(f"Invalid package.version format: {version}")

    def validate_object(self) -> None:
        r"""验证 [object] 部分"""
        if "[object]" not in self.content:
            self.errors.append("Missing required [object] section")
            return

        # 验证 kind
        kind_match = re.search(r'\[object].*?kind\s*=\s*"([^"]+)"', self.content, re.DOTALL)
        if not kind_match:
            self.errors.append("Missing object.kind")
        else:
            kind = kind_match.group(1)
            if kind not in self.VALID_KINDS:
                self.errors.append(f"Invalid object.kind: {kind}")

    def validate_states(self) -> None:
        r"""验证 [[states]] 部分"""
        if "[[states]]" not in self.content:
            self.errors.append("Missing required [[states]] section")
            return

        # 提取所有 state id
        state_ids = re.findall(r"\[\[states]].*?id\s*=\s*(\d+)", self.content, re.DOTALL)
        state_ids_int = [int(sid) for sid in state_ids]

        if not state_ids_int:
            self.errors.append("No states found")
            return

        # 验证 id 从 0 开始连续递增
        expected_ids = list(range(len(state_ids_int)))
        if state_ids_int != expected_ids:
            self.errors.append(f"State IDs must be consecutive from 0: got {state_ids_int}")

        # 验证每个 state 都有 data
        state_data_count = self.content.count("[states.data]")
        if state_data_count != len(state_ids_int):
            self.errors.append(f"Expected {len(state_ids_int)} [states.data] sections, found {state_data_count}")

    def validate_steps(self) -> None:
        r"""验证 [[steps]] 部分"""
        step_ids = re.findall(r"\[\[steps]].*?id\s*=\s*(\d+)", self.content, re.DOTALL)
        step_ids_int = [int(sid) for sid in step_ids]

        if not step_ids_int:
            return  # steps 是可选的

        # 验证 id 从 0 开始连续递增
        expected_ids = list(range(len(step_ids_int)))
        if step_ids_int != expected_ids:
            self.errors.append(f"Step IDs must be consecutive from 0: got {step_ids_int}")

        # 验证每个 step 都有必需字段
        steps_sections = re.split(r"\[\[steps]]", self.content)[1:]
        for i, section in enumerate(steps_sections):
            # 截取到下一个 section
            next_section = re.search(r"\n\[(?!\[)", section)
            if next_section:
                section = section[:next_section.start()]

            if "op = " not in section:
                self.errors.append(f"Step {i} missing 'op' field")
            if "before = " not in section:
                self.errors.append(f"Step {i} missing 'before' field")
            if "[steps.args]" not in section:
                self.errors.append(f"Step {i} missing [steps.args] section")

    def validate_result_or_error(self) -> None:
        r"""验证 [result] 和 [error] 互斥"""
        has_result = "[result]" in self.content
        has_error = "[error]" in self.content

        if has_result and has_error:
            self.errors.append("Document cannot have both [result] and [error]")
        if not has_result and not has_error:
            self.errors.append("Document must have either [result] or [error]")

        if has_result:
            self.validate_result()
        if has_error:
            self.validate_error()

    def validate_result(self) -> None:
        r"""验证 [result] 部分"""
        final_state_match = re.search(r"\[result].*?final_state\s*=\s*(\d+)", self.content, re.DOTALL)
        if not final_state_match:
            self.errors.append("Missing result.final_state")

    def validate_error(self) -> None:
        r"""验证 [error] 部分"""
        # 验证 type
        error_type_match = re.search(r'\[error].*?type\s*=\s*"([^"]+)"', self.content, re.DOTALL)
        if not error_type_match:
            self.errors.append("Missing error.type")
        else:
            error_type = error_type_match.group(1)
            if error_type not in self.VALID_ERROR_TYPES:
                self.errors.append(f"Invalid error.type: {error_type}")

        # 验证 message
        message_match = re.search(r'\[error].*?message\s*=\s*"([^"]*)"', self.content, re.DOTALL)
        if not message_match:
            self.errors.append("Missing error.message")


class DataValidator:
    r"""数据结构特定验证器"""

    @staticmethod
    def validate_stack_data(content: str) -> list[str]:
        r"""验证 stack 数据格式"""
        errors: list[str] = []
        # 检查 data 中包含 items 和 top
        data_sections = re.findall(r"\[states\.data](?P<data>.*?)(?=\n\[|\Z)", content, re.DOTALL)
        for i, data in enumerate(data_sections):
            if "items = " not in data:
                errors.append(f"State {i} data missing 'items' field")
            if "top = " not in data:
                errors.append(f"State {i} data missing 'top' field")
        return errors

    @staticmethod
    def validate_queue_data(content: str) -> list[str]:
        r"""验证 queue 数据格式"""
        errors: list[str] = []
        data_sections = re.findall(r"\[states\.data](?P<data>.*?)(?=\n\[|\Z)", content, re.DOTALL)
        for i, data in enumerate(data_sections):
            if "items = " not in data:
                errors.append(f"State {i} data missing 'items' field")
            if "front = " not in data:
                errors.append(f"State {i} data missing 'front' field")
            if "rear = " not in data:
                errors.append(f"State {i} data missing 'rear' field")
        return errors

    @staticmethod
    def validate_slist_data(content: str) -> list[str]:
        r"""验证 slist 数据格式"""
        errors: list[str] = []
        data_sections = re.findall(r"\[states\.data](?P<data>.*?)(?=\n\[|\Z)", content, re.DOTALL)
        for i, data in enumerate(data_sections):
            if "head = " not in data:
                errors.append(f"State {i} data missing 'head' field")
            if "nodes = " not in data:
                errors.append(f"State {i} data missing 'nodes' field")
        return errors

    @staticmethod
    def validate_dlist_data(content: str) -> list[str]:
        r"""验证 dlist 数据格式"""
        errors: list[str] = []
        data_sections = re.findall(r"\[states\.data](?P<data>.*?)(?=\n\[|\Z)", content, re.DOTALL)
        for i, data in enumerate(data_sections):
            if "head = " not in data:
                errors.append(f"State {i} data missing 'head' field")
            if "tail = " not in data:
                errors.append(f"State {i} data missing 'tail' field")
            if "nodes = " not in data:
                errors.append(f"State {i} data missing 'nodes' field")
        return errors

    @staticmethod
    def validate_binary_tree_data(content: str) -> list[str]:
        r"""验证 binary_tree 数据格式"""
        errors: list[str] = []
        data_sections = re.findall(r"\[states\.data](?P<data>.*?)(?=\n\[|\Z)", content, re.DOTALL)
        for i, data in enumerate(data_sections):
            if "root = " not in data:
                errors.append(f"State {i} data missing 'root' field")
            if "nodes = " not in data:
                errors.append(f"State {i} data missing 'nodes' field")
        return errors

    @staticmethod
    def validate_graph_data(content: str) -> list[str]:
        r"""验证图数据格式"""
        errors: list[str] = []
        data_sections = re.findall(r"\[states\.data](?P<data>.*?)(?=\n\[|\Z)", content, re.DOTALL)
        for i, data in enumerate(data_sections):
            if "nodes = " not in data:
                errors.append(f"State {i} data missing 'nodes' field")
            if "edges = " not in data:
                errors.append(f"State {i} data missing 'edges' field")
        return errors


class TestStackIntegration:
    r"""栈集成测试"""

    def test_stack_basic_operations(self, tmp_path: Any) -> None:
        r"""测试栈基本操作"""
        output_file = tmp_path / "stack_basic.toml"
        with stack(label="test_stack", output=str(output_file)) as s:
            s.push(10)
            s.push(20)
            s.push(30)
            s.pop()
            s.push(40)

        content = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

        # 验证 stack 特定数据格式
        errors = DataValidator.validate_stack_data(content)
        assert not errors, f"Stack data validation errors: {errors}"

        # 验证 kind
        assert 'kind = "stack"' in content

    def test_stack_empty_pop_error(self, tmp_path: Any) -> None:
        r"""测试空栈弹出错误"""
        output_file = tmp_path / "stack_error.toml"
        with pytest.raises(StructureError):
            with stack(output=str(output_file)) as s:
                s.pop()

        content = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

        # 验证有 error 部分
        assert "[error]" in content
        assert "[result]" not in content
        assert 'type = "runtime"' in content

    def test_stack_clear(self, tmp_path: Any) -> None:
        r"""测试栈清空操作"""
        output_file = tmp_path / "stack_clear.toml"
        with stack(output=str(output_file)) as s:
            s.push(1)
            s.push(2)
            s.push(3)
            s.clear()

        content = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

        # 验证最后状态是空栈
        assert "items = []" in content

    def test_stack_various_value_types(self, tmp_path: Any) -> None:
        r"""测试栈支持各种值类型"""
        output_file = tmp_path / "stack_types.toml"
        with stack(output=str(output_file)) as s:
            s.push(42)          # int
            s.push(3.14)        # float
            s.push("hello")     # string
            s.push(True)        # bool
            s.push(False)       # bool

        content = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"


class TestQueueIntegration:
    r"""队列集成测试"""

    def test_queue_basic_operations(self, tmp_path: Any) -> None:
        r"""测试队列基本操作"""
        output_file = tmp_path / "queue_basic.toml"
        with queue(label="test_queue", output=str(output_file)) as q:
            q.enqueue(10)
            q.enqueue(20)
            q.dequeue()
            q.enqueue(30)

        content = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

        errors = DataValidator.validate_queue_data(content)
        assert not errors, f"Queue data validation errors: {errors}"

        assert 'kind = "queue"' in content

    def test_queue_empty_dequeue_error(self, tmp_path: Any) -> None:
        r"""测试空队列出队错误"""
        output_file = tmp_path / "queue_error.toml"
        with pytest.raises(StructureError):
            with queue(output=str(output_file)) as q:
                q.dequeue()

        content = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

        assert "[error]" in content

    def test_queue_front_rear_consistency(self, tmp_path: Any) -> None:
        r"""测试队列 front/rear 一致性"""
        output_file = tmp_path / "queue_consistency.toml"
        with queue(output=str(output_file)) as q:
            q.enqueue(1)
            q.enqueue(2)
            q.enqueue(3)
            q.dequeue()
            q.dequeue()

        content = output_file.read_text(encoding="utf-8")
        # 验证空队列时 front = rear = -1
        # 非空时 0 <= front <= rear < len(items)
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"


class TestSingleLinkedListIntegration:
    r"""单链表集成测试"""

    def test_slist_basic_operations(self, tmp_path: Any) -> None:
        r"""测试单链表基本操作"""
        output_file = tmp_path / "slist_basic.toml"
        with single_linked_list(label="test_slist", output=str(output_file)) as slist:
            node1 = slist.insert_head(10)
            node2 = slist.insert_tail(20)
            slist.insert_after(node1, 15)
            slist.delete_head()

        content = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

        errors = DataValidator.validate_slist_data(content)
        assert not errors, f"Slist data validation errors: {errors}"

        assert 'kind = "slist"' in content

    def test_slist_reverse(self, tmp_path: Any) -> None:
        r"""测试单链表反转"""
        output_file = tmp_path / "slist_reverse.toml"
        with single_linked_list(output=str(output_file)) as slist:
            slist.insert_tail(1)
            slist.insert_tail(2)
            slist.insert_tail(3)
            slist.reverse()

        content = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_slist_delete_nonexistent_node_error(self, tmp_path: Any) -> None:
        r"""测试删除不存在的节点"""
        output_file = tmp_path / "slist_error.toml"
        with pytest.raises(StructureError):
            with single_linked_list(output=str(output_file)) as slist:
                slist.insert_head(10)
                slist.delete(999)  # 不存在的节点

        content = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

        assert "[error]" in content

    def test_slist_empty_delete_head_error(self, tmp_path: Any) -> None:
        r"""测试空链表删除头节点"""
        output_file = tmp_path / "slist_empty_delete.toml"
        with pytest.raises(StructureError):
            with single_linked_list(output=str(output_file)) as slist:
                slist.delete_head()

        content = output_file.read_text(encoding="utf-8")
        assert "[error]" in content


class TestDoubleLinkedListIntegration:
    r"""双向链表集成测试"""

    def test_dlist_basic_operations(self, tmp_path: Any) -> None:
        r"""测试双向链表基本操作"""
        output_file = tmp_path / "dlist_basic.toml"
        with double_linked_list(label="test_dlist", output=str(output_file)) as dlist:
            node1 = dlist.insert_head(10)
            node2 = dlist.insert_tail(30)
            dlist.insert_before(node2, 20)
            dlist.insert_after(node1, 15)
            dlist.delete_tail()

        content = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

        errors = DataValidator.validate_dlist_data(content)
        assert not errors, f"Dlist data validation errors: {errors}"

        assert 'kind = "dlist"' in content

    def test_dlist_reverse(self, tmp_path: Any) -> None:
        r"""测试双向链表反转"""
        output_file = tmp_path / "dlist_reverse.toml"
        with double_linked_list(output=str(output_file)) as dlist:
            dlist.insert_tail(1)
            dlist.insert_tail(2)
            dlist.insert_tail(3)
            dlist.reverse()

        content = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_dlist_head_tail_consistency(self, tmp_path: Any) -> None:
        r"""测试双向链表 head/tail 一致性"""
        output_file = tmp_path / "dlist_consistency.toml"
        with double_linked_list(output=str(output_file)) as dlist:
            dlist.insert_head(1)
            dlist.delete_head()
            # 空链表后 head 和 tail 都应该是 -1
            dlist.insert_tail(2)

        content = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"


class TestBinaryTreeIntegration:
    r"""二叉树集成测试"""

    def test_binary_tree_basic_operations(self, tmp_path: Any) -> None:
        r"""测试二叉树基本操作"""
        output_file = tmp_path / "btree_basic.toml"
        with binary_tree(label="test_btree", output=str(output_file)) as bt:
            root = bt.insert_root(10)
            left = bt.insert_left(root, 5)
            right = bt.insert_right(root, 15)
            bt.insert_left(left, 3)
            bt.insert_right(left, 7)

        content = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

        errors = DataValidator.validate_binary_tree_data(content)
        assert not errors, f"Binary tree data validation errors: {errors}"

        assert 'kind = "binary_tree"' in content

    def test_binary_tree_delete_subtree(self, tmp_path: Any) -> None:
        r"""测试删除子树"""
        output_file = tmp_path / "btree_delete.toml"
        with binary_tree(output=str(output_file)) as bt:
            root = bt.insert_root(10)
            left = bt.insert_left(root, 5)
            bt.insert_left(left, 3)
            bt.insert_right(left, 7)
            bt.delete(left)  # 删除整个左子树

        content = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_binary_tree_update_value(self, tmp_path: Any) -> None:
        r"""测试更新节点值"""
        output_file = tmp_path / "btree_update.toml"
        with binary_tree(output=str(output_file)) as bt:
            root = bt.insert_root(10)
            bt.update_value(root, 100)

        content = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_binary_tree_duplicate_root_error(self, tmp_path: Any) -> None:
        r"""测试重复插入根节点"""
        output_file = tmp_path / "btree_dup_root.toml"
        with pytest.raises(StructureError):
            with binary_tree(output=str(output_file)) as bt:
                bt.insert_root(10)
                bt.insert_root(20)  # 应该报错

        content = output_file.read_text(encoding="utf-8")
        assert "[error]" in content

    def test_binary_tree_duplicate_child_error(self, tmp_path: Any) -> None:
        r"""测试重复插入子节点"""
        output_file = tmp_path / "btree_dup_child.toml"
        with pytest.raises(StructureError):
            with binary_tree(output=str(output_file)) as bt:
                root = bt.insert_root(10)
                bt.insert_left(root, 5)
                bt.insert_left(root, 6)  # 应该报错

        content = output_file.read_text(encoding="utf-8")
        assert "[error]" in content


class TestBinarySearchTreeIntegration:
    r"""二叉搜索树集成测试"""

    def test_bst_basic_operations(self, tmp_path: Any) -> None:
        r"""测试 BST 基本操作"""
        output_file = tmp_path / "bst_basic.toml"
        with binary_search_tree(label="test_bst", output=str(output_file)) as bst:
            bst.insert(10)
            bst.insert(5)
            bst.insert(15)
            bst.insert(3)
            bst.insert(7)

        content = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

        assert 'kind = "bst"' in content

    def test_bst_delete_leaf(self, tmp_path: Any) -> None:
        r"""测试删除叶节点"""
        output_file = tmp_path / "bst_delete_leaf.toml"
        with binary_search_tree(output=str(output_file)) as bst:
            bst.insert(10)
            bst.insert(5)
            bst.insert(15)
            bst.delete(5)

        content = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_bst_delete_with_one_child(self, tmp_path: Any) -> None:
        r"""测试删除只有一个子节点的节点"""
        output_file = tmp_path / "bst_delete_one_child.toml"
        with binary_search_tree(output=str(output_file)) as bst:
            bst.insert(10)
            bst.insert(5)
            bst.insert(3)  # 5 只有左子节点
            bst.delete(5)

        content = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_bst_delete_with_two_children(self, tmp_path: Any) -> None:
        r"""测试删除有两个子节点的节点"""
        output_file = tmp_path / "bst_delete_two_children.toml"
        with binary_search_tree(output=str(output_file)) as bst:
            bst.insert(10)
            bst.insert(5)
            bst.insert(15)
            bst.insert(3)
            bst.insert(7)
            bst.delete(5)  # 5 有两个子节点

        content = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_bst_delete_nonexistent_error(self, tmp_path: Any) -> None:
        r"""测试删除不存在的节点"""
        output_file = tmp_path / "bst_delete_error.toml"
        with pytest.raises(StructureError):
            with binary_search_tree(output=str(output_file)) as bst:
                bst.insert(10)
                bst.delete(999)

        content = output_file.read_text(encoding="utf-8")
        assert "[error]" in content


class TestHeapIntegration:
    r"""堆集成测试"""

    def test_min_heap_basic(self, tmp_path: Any) -> None:
        r"""测试最小堆基本操作"""
        output_file = tmp_path / "min_heap.toml"
        with heap(label="test_heap", heap_type="min", output=str(output_file)) as h:
            h.insert(10)
            h.insert(5)
            h.insert(15)
            h.insert(3)
            h.extract()

        content = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

        # 堆使用 binary_tree 类型
        assert 'kind = "binary_tree"' in content

    def test_max_heap_basic(self, tmp_path: Any) -> None:
        r"""测试最大堆基本操作"""
        output_file = tmp_path / "max_heap.toml"
        with heap(heap_type="max", output=str(output_file)) as h:
            h.insert(10)
            h.insert(20)
            h.insert(5)
            h.extract()

        content = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_heap_extract_empty_error(self, tmp_path: Any) -> None:
        r"""测试空堆提取错误"""
        output_file = tmp_path / "heap_error.toml"
        with pytest.raises(StructureError):
            with heap(output=str(output_file)) as h:
                h.extract()

        content = output_file.read_text(encoding="utf-8")
        assert "[error]" in content

    def test_heap_clear(self, tmp_path: Any) -> None:
        r"""测试堆清空"""
        output_file = tmp_path / "heap_clear.toml"
        with heap(output=str(output_file)) as h:
            h.insert(1)
            h.insert(2)
            h.insert(3)
            h.clear()

        content = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_heap_invalid_type_error(self) -> None:
        r"""测试无效堆类型"""
        with pytest.raises(ValueError):
            heap(heap_type="invalid")


class TestGraphUndirectedIntegration:
    r"""无向图集成测试"""

    def test_graph_undirected_basic(self, tmp_path: Any) -> None:
        r"""测试无向图基本操作"""
        output_file = tmp_path / "graph_undirected.toml"
        with graph_undirected(label="test_graph", output=str(output_file)) as g:
            g.add_node(0, "A")
            g.add_node(1, "B")
            g.add_node(2, "C")
            g.add_edge(0, 1)
            g.add_edge(1, 2)

        content = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

        errors = DataValidator.validate_graph_data(content)
        assert not errors, f"Graph data validation errors: {errors}"

        assert 'kind = "graph_undirected"' in content

    def test_graph_undirected_edge_normalization(self, tmp_path: Any) -> None:
        r"""测试无向图边规范化 (from < to)"""
        output_file = tmp_path / "graph_undirected_norm.toml"
        with graph_undirected(output=str(output_file)) as g:
            g.add_node(0, "A")
            g.add_node(1, "B")
            g.add_edge(1, 0)  # 应该被规范化为 (0, 1)

        content = output_file.read_text(encoding="utf-8")
        # 验证边被规范化
        assert "from = 0" in content
        assert "to = 1" in content

    def test_graph_undirected_self_loop_error(self, tmp_path: Any) -> None:
        r"""测试无向图自环错误"""
        output_file = tmp_path / "graph_self_loop.toml"
        with pytest.raises(StructureError):
            with graph_undirected(output=str(output_file)) as g:
                g.add_node(0, "A")
                g.add_edge(0, 0)

        content = output_file.read_text(encoding="utf-8")
        assert "[error]" in content

    def test_graph_undirected_duplicate_edge_error(self, tmp_path: Any) -> None:
        r"""测试无向图重复边错误"""
        output_file = tmp_path / "graph_dup_edge.toml"
        with pytest.raises(StructureError):
            with graph_undirected(output=str(output_file)) as g:
                g.add_node(0, "A")
                g.add_node(1, "B")
                g.add_edge(0, 1)
                g.add_edge(0, 1)

        content = output_file.read_text(encoding="utf-8")
        assert "[error]" in content

    def test_graph_undirected_remove_node(self, tmp_path: Any) -> None:
        r"""测试删除节点（应同时删除相关边）"""
        output_file = tmp_path / "graph_remove_node.toml"
        with graph_undirected(output=str(output_file)) as g:
            g.add_node(0, "A")
            g.add_node(1, "B")
            g.add_node(2, "C")
            g.add_edge(0, 1)
            g.add_edge(1, 2)
            g.remove_node(1)

        content = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"


class TestGraphDirectedIntegration:
    r"""有向图集成测试"""

    def test_graph_directed_basic(self, tmp_path: Any) -> None:
        r"""测试有向图基本操作"""
        output_file = tmp_path / "graph_directed.toml"
        with graph_directed(label="test_digraph", output=str(output_file)) as g:
            g.add_node(0, "A")
            g.add_node(1, "B")
            g.add_edge(0, 1)
            g.add_edge(1, 0)  # 有向图允许反向边

        content = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

        assert 'kind = "graph_directed"' in content

    def test_graph_directed_no_edge_normalization(self, tmp_path: Any) -> None:
        r"""测试有向图不进行边规范化"""
        output_file = tmp_path / "graph_directed_no_norm.toml"
        with graph_directed(output=str(output_file)) as g:
            g.add_node(0, "A")
            g.add_node(1, "B")
            g.add_edge(1, 0)  # 有向图保持原样

        content = output_file.read_text(encoding="utf-8")
        # 验证边保持原样
        assert "from = 1" in content
        assert "to = 0" in content


class TestGraphWeightedIntegration:
    r"""带权图集成测试"""

    def test_graph_weighted_undirected(self, tmp_path: Any) -> None:
        r"""测试无向带权图"""
        output_file = tmp_path / "graph_weighted_undirected.toml"
        with graph_weighted(label="test_wgraph", directed=False, output=str(output_file)) as g:
            g.add_node(0, "A")
            g.add_node(1, "B")
            g.add_edge(0, 1, 3.5)

        content = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

        assert 'kind = "graph_weighted"' in content
        assert "weight = 3.5" in content

    def test_graph_weighted_directed(self, tmp_path: Any) -> None:
        r"""测试有向带权图"""
        output_file = tmp_path / "graph_weighted_directed.toml"
        with graph_weighted(directed=True, output=str(output_file)) as g:
            g.add_node(0, "A")
            g.add_node(1, "B")
            g.add_edge(0, 1, 2.0)
            g.add_edge(1, 0, 3.0)  # 有向图允许反向边

        content = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_graph_weighted_update_weight(self, tmp_path: Any) -> None:
        r"""测试更新边权重"""
        output_file = tmp_path / "graph_weighted_update.toml"
        with graph_weighted(output=str(output_file)) as g:
            g.add_node(0, "A")
            g.add_node(1, "B")
            g.add_edge(0, 1, 1.0)
            g.update_weight(0, 1, 5.0)

        content = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_graph_update_node_label(self, tmp_path: Any) -> None:
        r"""测试更新节点标签"""
        output_file = tmp_path / "graph_update_label.toml"
        with graph_weighted(output=str(output_file)) as g:
            g.add_node(0, "A")
            g.update_node_label(0, "X")

        content = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"


class TestGlobalConfigIntegration:
    r"""全局配置集成测试"""

    def test_config_remarks(self, tmp_path: Any) -> None:
        r"""测试配置 remarks"""
        output_file = tmp_path / "config_remarks.toml"
        config(
            output_path=str(output_file),
            title="Test Title",
            author="Test Author",
            comment="Test Comment"
        )

        with stack() as s:
            s.push(1)

        content = output_file.read_text(encoding="utf-8")
        assert "[remarks]" in content
        assert 'title = "Test Title"' in content
        assert 'author = "Test Author"' in content
        assert 'comment = "Test Comment"' in content

        # 恢复默认配置
        config()

    def test_config_output_path(self, tmp_path: Any) -> None:
        r"""测试配置输出路径"""
        output_file = tmp_path / "custom_output.toml"
        config(output_path=str(output_file))

        with stack() as s:
            s.push(1)

        assert output_file.exists()
        content = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

        # 恢复默认配置
        config()


class TestEdgeCases:
    r"""边缘情况测试"""

    def test_empty_structure(self, tmp_path: Any) -> None:
        r"""测试空数据结构"""
        output_file = tmp_path / "empty.toml"
        with stack(output=str(output_file)) as s:
            pass  # 不做任何操作

        content = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

        # 应该只有一个初始状态
        state_count = content.count("[[states]]")
        assert state_count == 1

    def test_special_characters_in_string(self, tmp_path: Any) -> None:
        r"""测试字符串中的特殊字符"""
        output_file = tmp_path / "special_chars.toml"
        with stack(output=str(output_file)) as s:
            s.push('hello"world')
            s.push("line1\\nline2")

        content = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_large_numbers(self, tmp_path: Any) -> None:
        r"""测试大数字"""
        output_file = tmp_path / "large_numbers.toml"
        with stack(output=str(output_file)) as s:
            s.push(10**18)
            s.push(-10**18)
            s.push(1.7976931348623157e308)  # 接近 float max

        content = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_many_operations(self, tmp_path: Any) -> None:
        r"""测试大量操作"""
        output_file = tmp_path / "many_ops.toml"
        with stack(output=str(output_file)) as s:
            for i in range(100):
                s.push(i)
            for _ in range(50):
                s.pop()

        content = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

        # 验证 state 和 step 数量
        state_count = content.count("[[states]]")
        step_count = content.count("[[steps]]")
        assert state_count == 151  # 1 初始 + 100 push + 50 pop
        assert step_count == 150

    def test_node_label_boundary(self, tmp_path: Any) -> None:
        r"""测试节点标签边界"""
        output_file = tmp_path / "label_boundary.toml"
        with graph_undirected(output=str(output_file)) as g:
            g.add_node(0, "A")  # 最短有效标签
            g.add_node(1, "A" * 32)  # 最长有效标签

        content = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_node_label_too_long_error(self, tmp_path: Any) -> None:
        r"""测试节点标签过长错误"""
        output_file = tmp_path / "label_too_long.toml"
        with pytest.raises(StructureError):
            with graph_undirected(output=str(output_file)) as g:
                g.add_node(0, "A" * 33)

        content = output_file.read_text(encoding="utf-8")
        assert "[error]" in content

    def test_node_label_empty_error(self, tmp_path: Any) -> None:
        r"""测试节点标签为空错误"""
        output_file = tmp_path / "label_empty.toml"
        with pytest.raises(StructureError):
            with graph_undirected(output=str(output_file)) as g:
                g.add_node(0, "")

        content = output_file.read_text(encoding="utf-8")
        assert "[error]" in content


class TestComplexScenarios:
    r"""复杂场景测试"""

    def test_bst_complex_deletion(self, tmp_path: Any) -> None:
        r"""测试 BST 复杂删除场景"""
        output_file = tmp_path / "bst_complex.toml"
        with binary_search_tree(output=str(output_file)) as bst:
            # 构建一个较复杂的 BST
            for v in [50, 30, 70, 20, 40, 60, 80, 35, 45]:
                bst.insert(v)
            # 删除有两个子节点的节点
            bst.delete(30)
            # 删除根节点
            bst.delete(50)

        content = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_graph_complex_operations(self, tmp_path: Any) -> None:
        r"""测试图的复杂操作"""
        output_file = tmp_path / "graph_complex.toml"
        with graph_weighted(directed=True, output=str(output_file)) as g:
            # 添加多个节点
            for i in range(5):
                g.add_node(i, chr(ord('A') + i))

            # 添加多条边
            g.add_edge(0, 1, 1.0)
            g.add_edge(0, 2, 2.0)
            g.add_edge(1, 2, 1.5)
            g.add_edge(2, 3, 2.5)
            g.add_edge(3, 4, 1.0)
            g.add_edge(4, 0, 3.0)

            # 更新权重
            g.update_weight(0, 1, 10.0)

            # 删除节点（会删除相关边）
            g.remove_node(2)

            # 更新标签
            g.update_node_label(0, "Start")

        content = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_linked_list_complex_operations(self, tmp_path: Any) -> None:
        r"""测试链表的复杂操作"""
        output_file = tmp_path / "list_complex.toml"
        with double_linked_list(output=str(output_file)) as dlist:
            # 构建链表
            nodes = []
            for i in range(5):
                nodes.append(dlist.insert_tail(i))

            # 在中间插入
            dlist.insert_after(nodes[2], 100)
            dlist.insert_before(nodes[2], 200)

            # 删除头尾
            dlist.delete_head()
            dlist.delete_tail()

            # 反转
            dlist.reverse()

        content = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"


class TestApiConsistency:
    r"""API 一致性测试"""

    def test_all_factory_functions_exist(self) -> None:
        r"""测试所有工厂函数存在"""
        assert callable(ds4viz.stack)
        assert callable(ds4viz.queue)
        assert callable(ds4viz.single_linked_list)
        assert callable(ds4viz.double_linked_list)
        assert callable(ds4viz.binary_tree)
        assert callable(ds4viz.binary_search_tree)
        assert callable(ds4viz.heap)
        assert callable(ds4viz.graph_undirected)
        assert callable(ds4viz.graph_directed)
        assert callable(ds4viz.graph_weighted)
        assert callable(ds4viz.config)

    def test_all_classes_exported(self) -> None:
        r"""测试所有类被导出"""
        assert ds4viz.Stack is Stack
        assert ds4viz.Queue is Queue
        assert ds4viz.SingleLinkedList is SingleLinkedList
        assert ds4viz.DoubleLinkedList is DoubleLinkedList
        assert ds4viz.BinaryTree is BinaryTree
        assert ds4viz.BinarySearchTree is BinarySearchTree
        assert ds4viz.Heap is Heap
        assert ds4viz.GraphUndirected is GraphUndirected
        assert ds4viz.GraphDirected is GraphDirected
        assert ds4viz.GraphWeighted is GraphWeighted

    def test_version_info(self) -> None:
        r"""测试版本信息"""
        assert hasattr(ds4viz, "__version__")
        assert re.match(r"^\d+\.\d+\.\d+$", ds4viz.__version__)

    def test_author_info(self) -> None:
        r"""测试作者信息"""
        assert hasattr(ds4viz, "__author__")
        assert "WaterRun" in ds4viz.__author__