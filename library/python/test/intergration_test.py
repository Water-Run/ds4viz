r"""
集成测试模块 - 完整的端到端测试

:file: test/integration_test.py
:author: WaterRun
:time: 2025-12-23
"""

import re
from pathlib import Path
from typing import Any, List, Set, Dict, Optional

import pytest

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

    VALID_KINDS: Set[str] = {
        "stack", "queue", "slist", "dlist",
        "binary_tree", "bst",
        "graph_undirected", "graph_directed", "graph_weighted"
    }

    VALID_ERROR_TYPES: Set[str] = {
        "runtime", "timeout", "validation", "sandbox", "unknown"
    }

    VALID_LANGS: Set[str] = {
        "python", "c", "zig", "rust", "java", "csharp", "typescript", "lua"
    }

    def __init__(self, content: str) -> None:
        r"""
        初始化验证器

        :param content: TOML 文件内容
        :return None: 无返回值
        """
        self.content: str = content
        self.lines: List[str] = content.split("\n")
        self.errors: List[str] = []

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
        r"""
        验证顶层结构顺序

        :return None: 无返回值
        """
        sections: List[str] = []
        for line in self.lines:
            line = line.strip()
            if line.startswith("[") and not line.startswith("[["):
                match = re.match(r"\[(?P<name>[a-z_.]+)]", line)
                if match:
                    sections.append(match.group("name"))
            elif line.startswith("[["):
                match = re.match(r"\[\[(?P<name>[a-z]+)]]", line)
                if match:
                    section_name: str = match.group("name")
                    if section_name not in sections:
                        sections.append(section_name)

        expected_order: List[str] = ["meta", "package", "remarks", "object", "states", "steps", "result", "error"]
        filtered_sections: List[str] = [
            s for s in sections
            if s in expected_order or s.split(".")[0] in expected_order
        ]

        seen: Set[str] = set()
        unique_sections: List[str] = []
        for s in filtered_sections:
            base: str = s.split(".")[0]
            if base not in seen:
                seen.add(base)
                unique_sections.append(base)

        prev_idx: int = -1
        for section in unique_sections:
            if section in expected_order:
                idx: int = expected_order.index(section)
                if idx < prev_idx:
                    self.errors.append(f"Section order violation: {section} should come earlier")
                prev_idx = idx

    def validate_meta(self) -> None:
        r"""
        验证 [meta] 部分

        :return None: 无返回值
        """
        if "[meta]" not in self.content:
            self.errors.append("Missing required [meta] section")
            return

        created_at_match = re.search(r'created_at\s*=\s*"(?P<timestamp>[^"]+)"', self.content)
        if not created_at_match:
            self.errors.append("Missing meta.created_at")
        else:
            created_at: str = created_at_match.group("timestamp")
            if not re.match(r"\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}Z", created_at):
                self.errors.append(f"Invalid meta.created_at format: {created_at}")

        lang_match = re.search(r'\[meta].*?lang\s*=\s*"(?P<lang>[^"]+)"', self.content, re.DOTALL)
        if not lang_match:
            self.errors.append("Missing meta.lang")
        else:
            lang: str = lang_match.group("lang")
            if lang not in self.VALID_LANGS:
                self.errors.append(f"Invalid meta.lang: {lang}")

        lang_version_match = re.search(r'lang_version\s*=\s*"(?P<version>[^"]+)"', self.content)
        if not lang_version_match:
            self.errors.append("Missing meta.lang_version")
        else:
            lang_version: str = lang_version_match.group("version")
            if not re.match(r"^[0-9a-z.+-]+$", lang_version):
                self.errors.append(f"Invalid meta.lang_version format: {lang_version}")

    def validate_package(self) -> None:
        r"""
        验证 [package] 部分

        :return None: 无返回值
        """
        if "[package]" not in self.content:
            self.errors.append("Missing required [package] section")
            return

        name_match = re.search(r'\[package].*?name\s*=\s*"(?P<name>[^"]+)"', self.content, re.DOTALL)
        if not name_match:
            self.errors.append("Missing package.name")
        else:
            name: str = name_match.group("name")
            if not re.match(r"^[0-9a-z_-]{1,64}$", name):
                self.errors.append(f"Invalid package.name: {name}")

        version_match = re.search(r'\[package].*?version\s*=\s*"(?P<version>[^"]+)"', self.content, re.DOTALL)
        if not version_match:
            self.errors.append("Missing package.version")
        else:
            version: str = version_match.group("version")
            if not re.match(r"^\d+\.\d+\.\d+$", version):
                self.errors.append(f"Invalid package.version format: {version}")

        pkg_lang_match = re.search(r'\[package].*?lang\s*=\s*"(?P<lang>[^"]+)"', self.content, re.DOTALL)
        meta_lang_match = re.search(r'\[meta].*?lang\s*=\s*"(?P<lang>[^"]+)"', self.content, re.DOTALL)
        if pkg_lang_match and meta_lang_match:
            if pkg_lang_match.group("lang") != meta_lang_match.group("lang"):
                self.errors.append("package.lang must equal meta.lang")

    def validate_object(self) -> None:
        r"""
        验证 [object] 部分

        :return None: 无返回值
        """
        if "[object]" not in self.content:
            self.errors.append("Missing required [object] section")
            return

        kind_match = re.search(r'\[object].*?kind\s*=\s*"(?P<kind>[^"]+)"', self.content, re.DOTALL)
        if not kind_match:
            self.errors.append("Missing object.kind")
        else:
            kind: str = kind_match.group("kind")
            if kind not in self.VALID_KINDS:
                self.errors.append(f"Invalid object.kind: {kind}")

    def validate_states(self) -> None:
        r"""
        验证 [[states]] 部分

        :return None: 无返回值
        """
        if "[[states]]" not in self.content:
            self.errors.append("Missing required [[states]] section")
            return

        state_ids: List[str] = re.findall(r"\[\[states]].*?id\s*=\s*(?P<id>\d+)", self.content, re.DOTALL)
        state_ids_int: List[int] = [int(sid) for sid in state_ids]

        if not state_ids_int:
            self.errors.append("No states found")
            return

        expected_ids: List[int] = list(range(len(state_ids_int)))
        if state_ids_int != expected_ids:
            self.errors.append(f"State IDs must be consecutive from 0: got {state_ids_int}")

        state_data_count: int = self.content.count("[states.data]")
        if state_data_count != len(state_ids_int):
            self.errors.append(f"Expected {len(state_ids_int)} [states.data] sections, found {state_data_count}")

    def validate_steps(self) -> None:
        r"""
        验证 [[steps]] 部分

        :return None: 无返回值
        """
        step_ids: List[str] = re.findall(r"\[\[steps]].*?id\s*=\s*(?P<id>\d+)", self.content, re.DOTALL)
        step_ids_int: List[int] = [int(sid) for sid in step_ids]

        if not step_ids_int:
            return

        expected_ids: List[int] = list(range(len(step_ids_int)))
        if step_ids_int != expected_ids:
            self.errors.append(f"Step IDs must be consecutive from 0: got {step_ids_int}")

        steps_sections: List[str] = re.split(r"\[\[steps]]", self.content)[1:]
        for i, section in enumerate(steps_sections):
            next_section = re.search(r"\n(?:\[\[steps]]|\[result]|\[error])", section)
            if next_section:
                section = section[:next_section.start()]

            if "op = " not in section:
                self.errors.append(f"Step {i} missing 'op' field")
            if "before = " not in section:
                self.errors.append(f"Step {i} missing 'before' field")
            if "[steps.args]" not in section:
                self.errors.append(f"Step {i} missing [steps.args] section")

    def validate_result_or_error(self) -> None:
        r"""
        验证 [result] 和 [error] 互斥

        :return None: 无返回值
        """
        has_result: bool = "[result]" in self.content
        has_error: bool = "[error]" in self.content

        if has_result and has_error:
            self.errors.append("Document cannot have both [result] and [error]")
        if not has_result and not has_error:
            self.errors.append("Document must have either [result] or [error]")

        if has_result:
            self.validate_result()
        if has_error:
            self.validate_error()

    def validate_result(self) -> None:
        r"""
        验证 [result] 部分

        :return None: 无返回值
        """
        final_state_match = re.search(r"\[result].*?final_state\s*=\s*(?P<state>\d+)", self.content, re.DOTALL)
        if not final_state_match:
            self.errors.append("Missing result.final_state")

    def validate_error(self) -> None:
        r"""
        验证 [error] 部分

        :return None: 无返回值
        """
        error_type_match = re.search(r'\[error].*?type\s*=\s*"(?P<type>[^"]+)"', self.content, re.DOTALL)
        if not error_type_match:
            self.errors.append("Missing error.type")
        else:
            error_type: str = error_type_match.group("type")
            if error_type not in self.VALID_ERROR_TYPES:
                self.errors.append(f"Invalid error.type: {error_type}")

        message_match = re.search(r'\[error].*?message\s*=\s*"(?P<message>[^"]*)"', self.content, re.DOTALL)
        if not message_match:
            self.errors.append("Missing error.message")


class DataValidator:
    r"""数据结构特定验证器"""

    @staticmethod
    def validate_stack_data(content: str) -> List[str]:
        r"""
        验证 stack 数据格式

        :param content: TOML 文件内容
        :return List[str]: 错误列表
        """
        errors: List[str] = []
        data_sections: List[str] = re.findall(r"\[states\.data](?P<data>.*?)(?=\n\[|\Z)", content, re.DOTALL)
        for i, data in enumerate(data_sections):
            if "items = " not in data:
                errors.append(f"State {i} data missing 'items' field")
            if "top = " not in data:
                errors.append(f"State {i} data missing 'top' field")

            items_match = re.search(r"items\s*=\s*\[(.*?)]", data)
            top_match = re.search(r"top\s*=\s*(-?\d+)", data)
            if items_match and top_match:
                items_str = items_match.group(1).strip()
                item_count = len([x for x in items_str.split(",") if x.strip()]) if items_str else 0
                top_val = int(top_match.group(1))
                if item_count == 0 and top_val != -1:
                    errors.append(f"State {i}: empty stack must have top = -1, got {top_val}")
                elif item_count > 0 and top_val != item_count - 1:
                    errors.append(f"State {i}: stack with {item_count} items must have top = {item_count - 1}, got {top_val}")

        return errors

    @staticmethod
    def validate_queue_data(content: str) -> List[str]:
        r"""
        验证 queue 数据格式

        :param content: TOML 文件内容
        :return List[str]: 错误列表
        """
        errors: List[str] = []
        data_sections: List[str] = re.findall(r"\[states\.data](?P<data>.*?)(?=\n\[|\Z)", content, re.DOTALL)
        for i, data in enumerate(data_sections):
            if "items = " not in data:
                errors.append(f"State {i} data missing 'items' field")
            if "front = " not in data:
                errors.append(f"State {i} data missing 'front' field")
            if "rear = " not in data:
                errors.append(f"State {i} data missing 'rear' field")

            items_match = re.search(r"items\s*=\s*\[(.*?)]", data)
            front_match = re.search(r"front\s*=\s*(-?\d+)", data)
            rear_match = re.search(r"rear\s*=\s*(-?\d+)", data)
            if items_match and front_match and rear_match:
                items_str = items_match.group(1).strip()
                item_count = len([x for x in items_str.split(",") if x.strip()]) if items_str else 0
                front_val = int(front_match.group(1))
                rear_val = int(rear_match.group(1))

                if item_count == 0:
                    if front_val != -1 or rear_val != -1:
                        errors.append(f"State {i}: empty queue must have front = -1 and rear = -1")
                else:
                    if not (0 <= front_val <= rear_val < item_count):
                        errors.append(f"State {i}: invalid front/rear values for non-empty queue")

        return errors

    @staticmethod
    def validate_slist_data(content: str) -> List[str]:
        r"""
        验证 slist 数据格式

        :param content: TOML 文件内容
        :return List[str]: 错误列表
        """
        errors: List[str] = []
        data_sections: List[str] = re.findall(r"\[states\.data](?P<data>.*?)(?=\n\[|\Z)", content, re.DOTALL)
        for i, data in enumerate(data_sections):
            if "head = " not in data:
                errors.append(f"State {i} data missing 'head' field")
            if "nodes = " not in data:
                errors.append(f"State {i} data missing 'nodes' field")
        return errors

    @staticmethod
    def validate_dlist_data(content: str) -> List[str]:
        r"""
        验证 dlist 数据格式

        :param content: TOML 文件内容
        :return List[str]: 错误列表
        """
        errors: List[str] = []
        data_sections: List[str] = re.findall(r"\[states\.data](?P<data>.*?)(?=\n\[|\Z)", content, re.DOTALL)
        for i, data in enumerate(data_sections):
            if "head = " not in data:
                errors.append(f"State {i} data missing 'head' field")
            if "tail = " not in data:
                errors.append(f"State {i} data missing 'tail' field")
            if "nodes = " not in data:
                errors.append(f"State {i} data missing 'nodes' field")

            head_match = re.search(r"head\s*=\s*(-?\d+)", data)
            tail_match = re.search(r"tail\s*=\s*(-?\d+)", data)
            if head_match and tail_match:
                head_val = int(head_match.group(1))
                tail_val = int(tail_match.group(1))
                if (head_val == -1 and tail_val != -1) or (head_val != -1 and tail_val == -1):
                    errors.append(f"State {i}: head and tail must be both -1 or both not -1")

        return errors

    @staticmethod
    def validate_binary_tree_data(content: str) -> List[str]:
        r"""
        验证 binary_tree 数据格式

        :param content: TOML 文件内容
        :return List[str]: 错误列表
        """
        errors: List[str] = []
        data_sections: List[str] = re.findall(r"\[states\.data](?P<data>.*?)(?=\n\[|\Z)", content, re.DOTALL)
        for i, data in enumerate(data_sections):
            if "root = " not in data:
                errors.append(f"State {i} data missing 'root' field")
            if "nodes = " not in data:
                errors.append(f"State {i} data missing 'nodes' field")
        return errors

    @staticmethod
    def validate_graph_data(content: str) -> List[str]:
        r"""
        验证图数据格式

        :param content: TOML 文件内容
        :return List[str]: 错误列表
        """
        errors: List[str] = []
        data_sections: List[str] = re.findall(r"\[states\.data](?P<data>.*?)(?=\n\[|\Z)", content, re.DOTALL)
        for i, data in enumerate(data_sections):
            if "nodes = " not in data:
                errors.append(f"State {i} data missing 'nodes' field")
            if "edges = " not in data:
                errors.append(f"State {i} data missing 'edges' field")
        return errors


class TestStackIntegration:
    r"""栈集成测试"""

    def test_stack_basic_operations(self, tmp_path: Path) -> None:
        r"""测试栈基本操作"""
        output_file = tmp_path / "stack_basic.toml"
        with stack(label="test_stack", output=str(output_file)) as s:
            s.push(10)
            s.push(20)
            s.push(30)
            s.pop()
            s.push(40)

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

        errors: List[str] = DataValidator.validate_stack_data(content)
        assert not errors, f"Stack data validation errors: {errors}"
        assert 'kind = "stack"' in content

    def test_stack_empty_pop_error(self, tmp_path: Path) -> None:
        r"""测试空栈弹出错误"""
        output_file = tmp_path / "stack_error.toml"
        with pytest.raises(StructureError):
            with stack(output=str(output_file)) as s:
                s.pop()

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"
        assert "[error]" in content
        assert "[result]" not in content
        assert 'type = "runtime"' in content

    def test_stack_clear(self, tmp_path: Path) -> None:
        r"""测试栈清空操作"""
        output_file = tmp_path / "stack_clear.toml"
        with stack(output=str(output_file)) as s:
            s.push(1)
            s.push(2)
            s.push(3)
            s.clear()

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"
        assert "items = []" in content

    def test_stack_various_value_types(self, tmp_path: Path) -> None:
        r"""测试栈支持各种值类型"""
        output_file = tmp_path / "stack_types.toml"
        with stack(output=str(output_file)) as s:
            s.push(42)
            s.push(3.14)
            s.push("hello")
            s.push(True)
            s.push(False)

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_stack_negative_numbers(self, tmp_path: Path) -> None:
        r"""测试栈处理负数"""
        output_file = tmp_path / "stack_negative.toml"
        with stack(output=str(output_file)) as s:
            s.push(-10)
            s.push(-20)
            s.pop()

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_stack_mixed_operations(self, tmp_path: Path) -> None:
        r"""测试栈混合操作"""
        output_file = tmp_path / "stack_mixed.toml"
        with stack(output=str(output_file)) as s:
            s.push(1)
            s.push(2)
            s.pop()
            s.push(3)
            s.clear()
            s.push(4)

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_stack_single_element(self, tmp_path: Path) -> None:
        r"""测试单元素栈"""
        output_file = tmp_path / "stack_single.toml"
        with stack(output=str(output_file)) as s:
            s.push(100)
            s.pop()

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_stack_push_after_clear(self, tmp_path: Path) -> None:
        r"""测试清空后再压入"""
        output_file = tmp_path / "stack_push_after_clear.toml"
        with stack(output=str(output_file)) as s:
            s.push(1)
            s.push(2)
            s.clear()
            s.push(10)
            s.push(20)

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_stack_multiple_pops(self, tmp_path: Path) -> None:
        r"""测试连续弹出"""
        output_file = tmp_path / "stack_multi_pop.toml"
        with stack(output=str(output_file)) as s:
            for i in range(5):
                s.push(i)
            for _ in range(3):
                s.pop()

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_stack_float_values(self, tmp_path: Path) -> None:
        r"""测试浮点数值"""
        output_file = tmp_path / "stack_float.toml"
        with stack(output=str(output_file)) as s:
            s.push(1.5)
            s.push(2.7)
            s.push(-3.14)
            s.pop()

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_stack_string_values(self, tmp_path: Path) -> None:
        r"""测试字符串值"""
        output_file = tmp_path / "stack_string.toml"
        with stack(output=str(output_file)) as s:
            s.push("first")
            s.push("second")
            s.push("third")
            s.pop()

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_stack_boolean_values(self, tmp_path: Path) -> None:
        r"""测试布尔值"""
        output_file = tmp_path / "stack_bool.toml"
        with stack(output=str(output_file)) as s:
            s.push(True)
            s.push(False)
            s.push(True)
            s.pop()

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_stack_zero_value(self, tmp_path: Path) -> None:
        r"""测试零值"""
        output_file = tmp_path / "stack_zero.toml"
        with stack(output=str(output_file)) as s:
            s.push(0)
            s.push(0.0)
            s.pop()

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"


class TestQueueIntegration:
    r"""队列集成测试"""

    def test_queue_basic_operations(self, tmp_path: Path) -> None:
        r"""测试队列基本操作"""
        output_file = tmp_path / "queue_basic.toml"
        with queue(label="test_queue", output=str(output_file)) as q:
            q.enqueue(10)
            q.enqueue(20)
            q.dequeue()
            q.enqueue(30)

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

        errors: List[str] = DataValidator.validate_queue_data(content)
        assert not errors, f"Queue data validation errors: {errors}"
        assert 'kind = "queue"' in content

    def test_queue_empty_dequeue_error(self, tmp_path: Path) -> None:
        r"""测试空队列出队错误"""
        output_file = tmp_path / "queue_error.toml"
        with pytest.raises(StructureError):
            with queue(output=str(output_file)) as q:
                q.dequeue()

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"
        assert "[error]" in content

    def test_queue_clear(self, tmp_path: Path) -> None:
        r"""测试队列清空"""
        output_file = tmp_path / "queue_clear.toml"
        with queue(output=str(output_file)) as q:
            q.enqueue(1)
            q.enqueue(2)
            q.enqueue(3)
            q.clear()

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_queue_front_rear_consistency(self, tmp_path: Path) -> None:
        r"""测试队列 front/rear 一致性"""
        output_file = tmp_path / "queue_consistency.toml"
        with queue(output=str(output_file)) as q:
            q.enqueue(1)
            q.enqueue(2)
            q.enqueue(3)
            q.dequeue()
            q.dequeue()

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

        errors: List[str] = DataValidator.validate_queue_data(content)
        assert not errors, f"Queue data validation errors: {errors}"

    def test_queue_single_element(self, tmp_path: Path) -> None:
        r"""测试单元素队列"""
        output_file = tmp_path / "queue_single.toml"
        with queue(output=str(output_file)) as q:
            q.enqueue(100)
            q.dequeue()

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_queue_enqueue_after_clear(self, tmp_path: Path) -> None:
        r"""测试清空后再入队"""
        output_file = tmp_path / "queue_enqueue_after_clear.toml"
        with queue(output=str(output_file)) as q:
            q.enqueue(1)
            q.enqueue(2)
            q.clear()
            q.enqueue(10)
            q.enqueue(20)

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_queue_multiple_dequeues(self, tmp_path: Path) -> None:
        r"""测试连续出队"""
        output_file = tmp_path / "queue_multi_dequeue.toml"
        with queue(output=str(output_file)) as q:
            for i in range(5):
                q.enqueue(i)
            for _ in range(3):
                q.dequeue()

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_queue_alternating_operations(self, tmp_path: Path) -> None:
        r"""测试交替入队出队"""
        output_file = tmp_path / "queue_alternating.toml"
        with queue(output=str(output_file)) as q:
            q.enqueue(1)
            q.enqueue(2)
            q.dequeue()
            q.enqueue(3)
            q.dequeue()
            q.enqueue(4)

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_queue_negative_numbers(self, tmp_path: Path) -> None:
        r"""测试队列处理负数"""
        output_file = tmp_path / "queue_negative.toml"
        with queue(output=str(output_file)) as q:
            q.enqueue(-10)
            q.enqueue(-20)
            q.dequeue()

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_queue_float_values(self, tmp_path: Path) -> None:
        r"""测试队列浮点数值"""
        output_file = tmp_path / "queue_float.toml"
        with queue(output=str(output_file)) as q:
            q.enqueue(1.5)
            q.enqueue(2.7)
            q.dequeue()

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_queue_string_values(self, tmp_path: Path) -> None:
        r"""测试队列字符串值"""
        output_file = tmp_path / "queue_string.toml"
        with queue(output=str(output_file)) as q:
            q.enqueue("first")
            q.enqueue("second")
            q.dequeue()

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_queue_dequeue_until_empty(self, tmp_path: Path) -> None:
        r"""测试出队直到空"""
        output_file = tmp_path / "queue_until_empty.toml"
        with queue(output=str(output_file)) as q:
            for i in range(3):
                q.enqueue(i)
            for _ in range(3):
                q.dequeue()

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"


class TestSingleLinkedListIntegration:
    r"""单链表集成测试"""

    def test_slist_basic_operations(self, tmp_path: Path) -> None:
        r"""测试单链表基本操作"""
        output_file = tmp_path / "slist_basic.toml"
        with single_linked_list(label="test_slist", output=str(output_file)) as slist:
            node1: int = slist.insert_head(10)
            slist.insert_tail(20)
            slist.insert_after(node1, 15)
            slist.delete_head()

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

        errors: List[str] = DataValidator.validate_slist_data(content)
        assert not errors, f"Slist data validation errors: {errors}"
        assert 'kind = "slist"' in content

    def test_slist_reverse(self, tmp_path: Path) -> None:
        r"""测试单链表反转"""
        output_file = tmp_path / "slist_reverse.toml"
        with single_linked_list(output=str(output_file)) as slist:
            slist.insert_tail(1)
            slist.insert_tail(2)
            slist.insert_tail(3)
            slist.reverse()

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_slist_delete_nonexistent_node_error(self, tmp_path: Path) -> None:
        r"""测试删除不存在的节点"""
        output_file = tmp_path / "slist_error.toml"
        with pytest.raises(StructureError):
            with single_linked_list(output=str(output_file)) as slist:
                slist.insert_head(10)
                slist.delete(999)

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"
        assert "[error]" in content

    def test_slist_empty_delete_head_error(self, tmp_path: Path) -> None:
        r"""测试空链表删除头节点"""
        output_file = tmp_path / "slist_empty_delete.toml"
        with pytest.raises(StructureError):
            with single_linked_list(output=str(output_file)) as slist:
                slist.delete_head()

        content: str = output_file.read_text(encoding="utf-8")
        assert "[error]" in content

    def test_slist_insert_after_nonexistent_error(self, tmp_path: Path) -> None:
        r"""测试在不存在的节点后插入"""
        output_file = tmp_path / "slist_insert_after_error.toml"
        with pytest.raises(StructureError):
            with single_linked_list(output=str(output_file)) as slist:
                slist.insert_head(10)
                slist.insert_after(999, 20)

        content: str = output_file.read_text(encoding="utf-8")
        assert "[error]" in content

    def test_slist_delete_tail(self, tmp_path: Path) -> None:
        r"""测试删除尾节点"""
        output_file = tmp_path / "slist_delete_tail.toml"
        with single_linked_list(output=str(output_file)) as slist:
            node1: int = slist.insert_tail(1)
            node2: int = slist.insert_tail(2)
            node3: int = slist.insert_tail(3)
            slist.delete(node3)

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_slist_empty_delete_tail_error(self, tmp_path: Path) -> None:
        r"""测试空链表删除头节点错误"""
        output_file = tmp_path / "slist_empty_delete_head.toml"
        with pytest.raises(StructureError):
            with single_linked_list(output=str(output_file)) as slist:
                slist.delete_head()

        content: str = output_file.read_text(encoding="utf-8")
        assert "[error]" in content

    def test_slist_clear(self, tmp_path: Path) -> None:
        r"""测试单链表清空"""
        output_file = tmp_path / "slist_clear.toml"
        with single_linked_list(output=str(output_file)) as slist:
            node1: int = slist.insert_tail(1)
            node2: int = slist.insert_tail(2)
            slist.delete_head()
            slist.delete_head()

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_slist_single_node(self, tmp_path: Path) -> None:
        r"""测试单节点链表"""
        output_file = tmp_path / "slist_single.toml"
        with single_linked_list(output=str(output_file)) as slist:
            slist.insert_head(100)
            slist.delete_head()

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_slist_reverse_empty(self, tmp_path: Path) -> None:
        r"""测试反转空链表"""
        output_file = tmp_path / "slist_reverse_empty.toml"
        with single_linked_list(output=str(output_file)) as slist:
            slist.reverse()

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_slist_reverse_single(self, tmp_path: Path) -> None:
        r"""测试反转单节点链表"""
        output_file = tmp_path / "slist_reverse_single.toml"
        with single_linked_list(output=str(output_file)) as slist:
            slist.insert_head(1)
            slist.reverse()

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_slist_multiple_inserts(self, tmp_path: Path) -> None:
        r"""测试多次插入"""
        output_file = tmp_path / "slist_multi_insert.toml"
        with single_linked_list(output=str(output_file)) as slist:
            for i in range(5):
                slist.insert_tail(i)

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"


class TestDoubleLinkedListIntegration:
    r"""双向链表集成测试"""

    def test_dlist_basic_operations(self, tmp_path: Path) -> None:
        r"""测试双向链表基本操作"""
        output_file = tmp_path / "dlist_basic.toml"
        with double_linked_list(label="test_dlist", output=str(output_file)) as dlist:
            node1: int = dlist.insert_head(10)
            node2: int = dlist.insert_tail(30)
            dlist.insert_before(node2, 20)
            dlist.insert_after(node1, 15)
            dlist.delete_tail()

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

        errors: List[str] = DataValidator.validate_dlist_data(content)
        assert not errors, f"Dlist data validation errors: {errors}"
        assert 'kind = "dlist"' in content

    def test_dlist_reverse(self, tmp_path: Path) -> None:
        r"""测试双向链表反转"""
        output_file = tmp_path / "dlist_reverse.toml"
        with double_linked_list(output=str(output_file)) as dlist:
            dlist.insert_tail(1)
            dlist.insert_tail(2)
            dlist.insert_tail(3)
            dlist.reverse()

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_dlist_head_tail_consistency(self, tmp_path: Path) -> None:
        r"""测试双向链表 head/tail 一致性"""
        output_file = tmp_path / "dlist_consistency.toml"
        with double_linked_list(output=str(output_file)) as dlist:
            dlist.insert_head(1)
            dlist.delete_head()
            dlist.insert_tail(2)

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

        errors: List[str] = DataValidator.validate_dlist_data(content)
        assert not errors, f"Dlist data validation errors: {errors}"

    def test_dlist_delete_errors(self, tmp_path: Path) -> None:
        r"""测试双向链表删除错误"""
        output_file = tmp_path / "dlist_delete_error.toml"
        with pytest.raises(StructureError):
            with double_linked_list(output=str(output_file)) as dlist:
                dlist.delete_head()

        content: str = output_file.read_text(encoding="utf-8")
        assert "[error]" in content

    def test_dlist_delete_tail_error(self, tmp_path: Path) -> None:
        r"""测试空链表删除尾节点错误"""
        output_file = tmp_path / "dlist_delete_tail_error.toml"
        with pytest.raises(StructureError):
            with double_linked_list(output=str(output_file)) as dlist:
                dlist.delete_tail()

        content: str = output_file.read_text(encoding="utf-8")
        assert "[error]" in content

    def test_dlist_insert_before_nonexistent_error(self, tmp_path: Path) -> None:
        r"""测试在不存在的节点前插入"""
        output_file = tmp_path / "dlist_insert_before_error.toml"
        with pytest.raises(StructureError):
            with double_linked_list(output=str(output_file)) as dlist:
                dlist.insert_head(10)
                dlist.insert_before(999, 20)

        content: str = output_file.read_text(encoding="utf-8")
        assert "[error]" in content

    def test_dlist_insert_after_nonexistent_error(self, tmp_path: Path) -> None:
        r"""测试在不存在的节点后插入"""
        output_file = tmp_path / "dlist_insert_after_error.toml"
        with pytest.raises(StructureError):
            with double_linked_list(output=str(output_file)) as dlist:
                dlist.insert_head(10)
                dlist.insert_after(999, 20)

        content: str = output_file.read_text(encoding="utf-8")
        assert "[error]" in content

    def test_dlist_delete_nonexistent_error(self, tmp_path: Path) -> None:
        r"""测试删除不存在的节点"""
        output_file = tmp_path / "dlist_delete_node_error.toml"
        with pytest.raises(StructureError):
            with double_linked_list(output=str(output_file)) as dlist:
                dlist.insert_head(10)
                dlist.delete(999)

        content: str = output_file.read_text(encoding="utf-8")
        assert "[error]" in content

    def test_dlist_clear(self, tmp_path: Path) -> None:
        r"""测试双向链表清空"""
        output_file = tmp_path / "dlist_clear.toml"
        with double_linked_list(output=str(output_file)) as dlist:
            dlist.insert_tail(1)
            dlist.insert_tail(2)
            dlist.delete_head()
            dlist.delete_head()

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_dlist_single_node(self, tmp_path: Path) -> None:
        r"""测试单节点双向链表"""
        output_file = tmp_path / "dlist_single.toml"
        with double_linked_list(output=str(output_file)) as dlist:
            dlist.insert_head(100)
            dlist.delete_head()

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_dlist_reverse_empty(self, tmp_path: Path) -> None:
        r"""测试反转空链表"""
        output_file = tmp_path / "dlist_reverse_empty.toml"
        with double_linked_list(output=str(output_file)) as dlist:
            dlist.reverse()

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_dlist_multiple_operations(self, tmp_path: Path) -> None:
        r"""测试多种操作组合"""
        output_file = tmp_path / "dlist_multi_ops.toml"
        with double_linked_list(output=str(output_file)) as dlist:
            n1: int = dlist.insert_head(1)
            n2: int = dlist.insert_tail(5)
            dlist.insert_after(n1, 2)
            dlist.insert_before(n2, 4)
            dlist.delete(n1)

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"


class TestBinaryTreeIntegration:
    r"""二叉树集成测试"""

    def test_binary_tree_basic_operations(self, tmp_path: Path) -> None:
        r"""测试二叉树基本操作"""
        output_file = tmp_path / "btree_basic.toml"
        with binary_tree(label="test_btree", output=str(output_file)) as bt:
            root: int = bt.insert_root(10)
            left: int = bt.insert_left(root, 5)
            bt.insert_right(root, 15)
            bt.insert_left(left, 3)
            bt.insert_right(left, 7)

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

        errors: List[str] = DataValidator.validate_binary_tree_data(content)
        assert not errors, f"Binary tree data validation errors: {errors}"
        assert 'kind = "binary_tree"' in content

    def test_binary_tree_delete_subtree(self, tmp_path: Path) -> None:
        r"""测试删除子树"""
        output_file = tmp_path / "btree_delete.toml"
        with binary_tree(output=str(output_file)) as bt:
            root: int = bt.insert_root(10)
            left: int = bt.insert_left(root, 5)
            bt.insert_left(left, 3)
            bt.insert_right(left, 7)
            bt.delete(left)

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_binary_tree_update_value(self, tmp_path: Path) -> None:
        r"""测试更新节点值"""
        output_file = tmp_path / "btree_update.toml"
        with binary_tree(output=str(output_file)) as bt:
            root: int = bt.insert_root(10)
            bt.update_value(root, 100)

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_binary_tree_duplicate_root_error(self, tmp_path: Path) -> None:
        r"""测试重复插入根节点"""
        output_file = tmp_path / "btree_dup_root.toml"
        with pytest.raises(StructureError):
            with binary_tree(output=str(output_file)) as bt:
                bt.insert_root(10)
                bt.insert_root(20)

        content: str = output_file.read_text(encoding="utf-8")
        assert "[error]" in content

    def test_binary_tree_duplicate_child_error(self, tmp_path: Path) -> None:
        r"""测试重复插入子节点"""
        output_file = tmp_path / "btree_dup_child.toml"
        with pytest.raises(StructureError):
            with binary_tree(output=str(output_file)) as bt:
                root: int = bt.insert_root(10)
                bt.insert_left(root, 5)
                bt.insert_left(root, 6)

        content: str = output_file.read_text(encoding="utf-8")
        assert "[error]" in content

    def test_binary_tree_nonexistent_parent_error(self, tmp_path: Path) -> None:
        r"""测试不存在的父节点"""
        output_file = tmp_path / "btree_no_parent.toml"
        with pytest.raises(StructureError):
            with binary_tree(output=str(output_file)) as bt:
                bt.insert_root(10)
                bt.insert_left(999, 5)

        content: str = output_file.read_text(encoding="utf-8")
        assert "[error]" in content

    def test_binary_tree_delete_root(self, tmp_path: Path) -> None:
        r"""测试删除根节点"""
        output_file = tmp_path / "btree_delete_root.toml"
        with binary_tree(output=str(output_file)) as bt:
            root: int = bt.insert_root(10)
            bt.insert_left(root, 5)
            bt.delete(root)

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_binary_tree_delete_nonexistent_error(self, tmp_path: Path) -> None:
        r"""测试删除不存在的节点"""
        output_file = tmp_path / "btree_delete_nonexist.toml"
        with pytest.raises(StructureError):
            with binary_tree(output=str(output_file)) as bt:
                bt.insert_root(10)
                bt.delete(999)

        content: str = output_file.read_text(encoding="utf-8")
        assert "[error]" in content

    def test_binary_tree_update_nonexistent_error(self, tmp_path: Path) -> None:
        r"""测试更新不存在的节点"""
        output_file = tmp_path / "btree_update_nonexist.toml"
        with pytest.raises(StructureError):
            with binary_tree(output=str(output_file)) as bt:
                bt.insert_root(10)
                bt.update_value(999, 100)

        content: str = output_file.read_text(encoding="utf-8")
        assert "[error]" in content

    def test_binary_tree_clear(self, tmp_path: Path) -> None:
        r"""测试二叉树清空"""
        output_file = tmp_path / "btree_clear.toml"
        with binary_tree(output=str(output_file)) as bt:
            root: int = bt.insert_root(10)
            bt.insert_left(root, 5)
            bt.delete(root)

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_binary_tree_complete_tree(self, tmp_path: Path) -> None:
        r"""测试完全二叉树"""
        output_file = tmp_path / "btree_complete.toml"
        with binary_tree(output=str(output_file)) as bt:
            root: int = bt.insert_root(1)
            left: int = bt.insert_left(root, 2)
            right: int = bt.insert_right(root, 3)
            bt.insert_left(left, 4)
            bt.insert_right(left, 5)
            bt.insert_left(right, 6)
            bt.insert_right(right, 7)

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"


class TestBinarySearchTreeIntegration:
    r"""二叉搜索树集成测试"""

    def test_bst_basic_operations(self, tmp_path: Path) -> None:
        r"""测试 BST 基本操作"""
        output_file = tmp_path / "bst_basic.toml"
        with binary_search_tree(label="test_bst", output=str(output_file)) as bst:
            bst.insert(10)
            bst.insert(5)
            bst.insert(15)
            bst.insert(3)
            bst.insert(7)

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"
        assert 'kind = "bst"' in content

    def test_bst_delete_leaf(self, tmp_path: Path) -> None:
        r"""测试删除叶节点"""
        output_file = tmp_path / "bst_delete_leaf.toml"
        with binary_search_tree(output=str(output_file)) as bst:
            bst.insert(10)
            bst.insert(5)
            bst.insert(15)
            bst.delete(5)

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_bst_delete_with_one_child(self, tmp_path: Path) -> None:
        r"""测试删除只有一个子节点的节点"""
        output_file = tmp_path / "bst_delete_one_child.toml"
        with binary_search_tree(output=str(output_file)) as bst:
            bst.insert(10)
            bst.insert(5)
            bst.insert(3)
            bst.delete(5)

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_bst_delete_with_two_children(self, tmp_path: Path) -> None:
        r"""测试删除有两个子节点的节点"""
        output_file = tmp_path / "bst_delete_two_children.toml"
        with binary_search_tree(output=str(output_file)) as bst:
            bst.insert(10)
            bst.insert(5)
            bst.insert(15)
            bst.insert(3)
            bst.insert(7)
            bst.delete(5)

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_bst_delete_nonexistent_error(self, tmp_path: Path) -> None:
        r"""测试删除不存在的节点"""
        output_file = tmp_path / "bst_delete_error.toml"
        with pytest.raises(StructureError):
            with binary_search_tree(output=str(output_file)) as bst:
                bst.insert(10)
                bst.delete(999)

        content: str = output_file.read_text(encoding="utf-8")
        assert "[error]" in content

    def test_bst_delete_root(self, tmp_path: Path) -> None:
        r"""测试删除根节点"""
        output_file = tmp_path / "bst_delete_root.toml"
        with binary_search_tree(output=str(output_file)) as bst:
            bst.insert(10)
            bst.insert(5)
            bst.insert(15)
            bst.delete(10)

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_bst_insert_duplicate(self, tmp_path: Path) -> None:
        r"""测试插入重复值"""
        output_file = tmp_path / "bst_duplicate.toml"
        with binary_search_tree(output=str(output_file)) as bst:
            bst.insert(10)
            bst.insert(5)
            bst.insert(10)

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_bst_clear(self, tmp_path: Path) -> None:
        r"""测试二叉搜索树清空"""
        output_file = tmp_path / "bst_clear.toml"
        with binary_search_tree(output=str(output_file)) as bst:
            bst.insert(10)
            bst.insert(5)
            bst.delete(10)
            bst.delete(5)

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_bst_left_skewed_tree(self, tmp_path: Path) -> None:
        r"""测试左偏树"""
        output_file = tmp_path / "bst_left_skewed.toml"
        with binary_search_tree(output=str(output_file)) as bst:
            for i in range(5, 0, -1):
                bst.insert(i)

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_bst_right_skewed_tree(self, tmp_path: Path) -> None:
        r"""测试右偏树"""
        output_file = tmp_path / "bst_right_skewed.toml"
        with binary_search_tree(output=str(output_file)) as bst:
            for i in range(1, 6):
                bst.insert(i)

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"


class TestHeapIntegration:
    r"""堆集成测试"""

    def test_min_heap_basic(self, tmp_path: Path) -> None:
        r"""测试最小堆基本操作"""
        output_file = tmp_path / "min_heap.toml"
        with heap(label="test_heap", heap_type="min", output=str(output_file)) as h:
            h.insert(10)
            h.insert(5)
            h.insert(15)
            h.insert(3)
            h.extract()

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"
        assert 'kind = "binary_tree"' in content

    def test_max_heap_basic(self, tmp_path: Path) -> None:
        r"""测试最大堆基本操作"""
        output_file = tmp_path / "max_heap.toml"
        with heap(heap_type="max", output=str(output_file)) as h:
            h.insert(10)
            h.insert(20)
            h.insert(5)
            h.extract()

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_heap_extract_empty_error(self, tmp_path: Path) -> None:
        r"""测试空堆提取错误"""
        output_file = tmp_path / "heap_error.toml"
        with pytest.raises(StructureError):
            with heap(output=str(output_file)) as h:
                h.extract()

        content: str = output_file.read_text(encoding="utf-8")
        assert "[error]" in content

    def test_heap_clear(self, tmp_path: Path) -> None:
        r"""测试堆清空"""
        output_file = tmp_path / "heap_clear.toml"
        with heap(output=str(output_file)) as h:
            h.insert(1)
            h.insert(2)
            h.insert(3)
            h.clear()

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_heap_invalid_type_error(self) -> None:
        r"""测试无效堆类型"""
        with pytest.raises(ValueError):
            _ = heap(heap_type="invalid")

    def test_heap_multiple_extract(self, tmp_path: Path) -> None:
        r"""测试多次提取"""
        output_file = tmp_path / "heap_multi_extract.toml"
        with heap(heap_type="min", output=str(output_file)) as h:
            h.insert(10)
            h.insert(5)
            h.insert(15)
            h.insert(3)
            h.extract()
            h.extract()

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_heap_single_element(self, tmp_path: Path) -> None:
        r"""测试单元素堆"""
        output_file = tmp_path / "heap_single.toml"
        with heap(output=str(output_file)) as h:
            h.insert(100)
            h.extract()

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_heap_insert_after_extract(self, tmp_path: Path) -> None:
        r"""测试提取后再插入"""
        output_file = tmp_path / "heap_insert_after_extract.toml"
        with heap(heap_type="min", output=str(output_file)) as h:
            h.insert(10)
            h.insert(5)
            h.extract()
            h.insert(3)

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_min_heap_property(self, tmp_path: Path) -> None:
        r"""测试最小堆性质"""
        output_file = tmp_path / "min_heap_property.toml"
        with heap(heap_type="min", output=str(output_file)) as h:
            values: List[int] = [15, 10, 20, 8, 25, 12]
            for v in values:
                h.insert(v)

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_max_heap_property(self, tmp_path: Path) -> None:
        r"""测试最大堆性质"""
        output_file = tmp_path / "max_heap_property.toml"
        with heap(heap_type="max", output=str(output_file)) as h:
            values: List[int] = [15, 10, 20, 8, 25, 12]
            for v in values:
                h.insert(v)

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"


class TestGraphUndirectedIntegration:
    r"""无向图集成测试"""

    def test_graph_undirected_basic(self, tmp_path: Path) -> None:
        r"""测试无向图基本操作"""
        output_file = tmp_path / "graph_undirected.toml"
        with graph_undirected(label="test_graph", output=str(output_file)) as g:
            g.add_node(0, "A")
            g.add_node(1, "B")
            g.add_node(2, "C")
            g.add_edge(0, 1)
            g.add_edge(1, 2)

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

        errors: List[str] = DataValidator.validate_graph_data(content)
        assert not errors, f"Graph data validation errors: {errors}"
        assert 'kind = "graph_undirected"' in content

    def test_graph_undirected_edge_normalization(self, tmp_path: Path) -> None:
        r"""测试无向图边规范化 (from < to)"""
        output_file = tmp_path / "graph_undirected_norm.toml"
        with graph_undirected(output=str(output_file)) as g:
            g.add_node(0, "A")
            g.add_node(1, "B")
            g.add_edge(1, 0)

        content: str = output_file.read_text(encoding="utf-8")
        assert "from = 0" in content
        assert "to = 1" in content

    def test_graph_undirected_self_loop_error(self, tmp_path: Path) -> None:
        r"""测试无向图自环错误"""
        output_file = tmp_path / "graph_self_loop.toml"
        with pytest.raises(StructureError):
            with graph_undirected(output=str(output_file)) as g:
                g.add_node(0, "A")
                g.add_edge(0, 0)

        content: str = output_file.read_text(encoding="utf-8")
        assert "[error]" in content

    def test_graph_undirected_duplicate_edge_error(self, tmp_path: Path) -> None:
        r"""测试无向图重复边错误"""
        output_file = tmp_path / "graph_dup_edge.toml"
        with pytest.raises(StructureError):
            with graph_undirected(output=str(output_file)) as g:
                g.add_node(0, "A")
                g.add_node(1, "B")
                g.add_edge(0, 1)
                g.add_edge(0, 1)

        content: str = output_file.read_text(encoding="utf-8")
        assert "[error]" in content

    def test_graph_undirected_remove_node(self, tmp_path: Path) -> None:
        r"""测试删除节点（应同时删除相关边）"""
        output_file = tmp_path / "graph_remove_node.toml"
        with graph_undirected(output=str(output_file)) as g:
            g.add_node(0, "A")
            g.add_node(1, "B")
            g.add_node(2, "C")
            g.add_edge(0, 1)
            g.add_edge(1, 2)
            g.remove_node(1)

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_graph_undirected_remove_edge(self, tmp_path: Path) -> None:
        r"""测试删除边"""
        output_file = tmp_path / "graph_remove_edge.toml"
        with graph_undirected(output=str(output_file)) as g:
            g.add_node(0, "A")
            g.add_node(1, "B")
            g.add_edge(0, 1)
            g.remove_edge(0, 1)

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_graph_undirected_add_node_duplicate_error(self, tmp_path: Path) -> None:
        r"""测试添加重复节点错误"""
        output_file = tmp_path / "graph_dup_node.toml"
        with pytest.raises(StructureError):
            with graph_undirected(output=str(output_file)) as g:
                g.add_node(0, "A")
                g.add_node(0, "B")

        content: str = output_file.read_text(encoding="utf-8")
        assert "[error]" in content

    def test_graph_undirected_remove_nonexistent_node_error(self, tmp_path: Path) -> None:
        r"""测试删除不存在的节点错误"""
        output_file = tmp_path / "graph_remove_nonexist_node.toml"
        with pytest.raises(StructureError):
            with graph_undirected(output=str(output_file)) as g:
                g.add_node(0, "A")
                g.remove_node(999)

        content: str = output_file.read_text(encoding="utf-8")
        assert "[error]" in content

    def test_graph_undirected_remove_nonexistent_edge_error(self, tmp_path: Path) -> None:
        r"""测试删除不存在的边错误"""
        output_file = tmp_path / "graph_remove_nonexist_edge.toml"
        with pytest.raises(StructureError):
            with graph_undirected(output=str(output_file)) as g:
                g.add_node(0, "A")
                g.add_node(1, "B")
                g.remove_edge(0, 1)

        content: str = output_file.read_text(encoding="utf-8")
        assert "[error]" in content

    def test_graph_undirected_clear(self, tmp_path: Path) -> None:
        r"""测试无向图清空"""
        output_file = tmp_path / "graph_undirected_clear.toml"
        with graph_undirected(output=str(output_file)) as g:
            g.add_node(0, "A")
            g.add_node(1, "B")
            g.remove_node(0)
            g.remove_node(1)

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"


class TestGraphDirectedIntegration:
    r"""有向图集成测试"""

    def test_graph_directed_basic(self, tmp_path: Path) -> None:
        r"""测试有向图基本操作"""
        output_file = tmp_path / "graph_directed.toml"
        with graph_directed(label="test_digraph", output=str(output_file)) as g:
            g.add_node(0, "A")
            g.add_node(1, "B")
            g.add_edge(0, 1)
            g.add_edge(1, 0)

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"
        assert 'kind = "graph_directed"' in content

    def test_graph_directed_no_edge_normalization(self, tmp_path: Path) -> None:
        r"""测试有向图不进行边规范化"""
        output_file = tmp_path / "graph_directed_no_norm.toml"
        with graph_directed(output=str(output_file)) as g:
            g.add_node(0, "A")
            g.add_node(1, "B")
            g.add_edge(1, 0)

        content: str = output_file.read_text(encoding="utf-8")
        assert "from = 1" in content
        assert "to = 0" in content

    def test_graph_directed_self_loop_error(self, tmp_path: Path) -> None:
        r"""测试有向图自环错误"""
        output_file = tmp_path / "graph_directed_self.toml"
        with pytest.raises(StructureError):
            with graph_directed(output=str(output_file)) as g:
                g.add_node(0, "A")
                g.add_edge(0, 0)

        content: str = output_file.read_text(encoding="utf-8")
        assert "[error]" in content

    def test_graph_directed_duplicate_edge_error(self, tmp_path: Path) -> None:
        r"""测试有向图重复边错误"""
        output_file = tmp_path / "graph_directed_dup_edge.toml"
        with pytest.raises(StructureError):
            with graph_directed(output=str(output_file)) as g:
                g.add_node(0, "A")
                g.add_node(1, "B")
                g.add_edge(0, 1)
                g.add_edge(0, 1)

        content: str = output_file.read_text(encoding="utf-8")
        assert "[error]" in content

    def test_graph_directed_remove_node(self, tmp_path: Path) -> None:
        r"""测试删除节点"""
        output_file = tmp_path / "graph_directed_remove_node.toml"
        with graph_directed(output=str(output_file)) as g:
            g.add_node(0, "A")
            g.add_node(1, "B")
            g.add_node(2, "C")
            g.add_edge(0, 1)
            g.add_edge(1, 2)
            g.remove_node(1)

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_graph_directed_remove_edge(self, tmp_path: Path) -> None:
        r"""测试删除边"""
        output_file = tmp_path / "graph_directed_remove_edge.toml"
        with graph_directed(output=str(output_file)) as g:
            g.add_node(0, "A")
            g.add_node(1, "B")
            g.add_edge(0, 1)
            g.remove_edge(0, 1)

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_graph_directed_clear(self, tmp_path: Path) -> None:
        r"""测试有向图清空"""
        output_file = tmp_path / "graph_directed_clear.toml"
        with graph_directed(output=str(output_file)) as g:
            g.add_node(0, "A")
            g.add_node(1, "B")
            g.remove_node(0)
            g.remove_node(1)

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"


class TestGraphWeightedIntegration:
    r"""带权图集成测试"""

    def test_graph_weighted_undirected(self, tmp_path: Path) -> None:
        r"""测试无向带权图"""
        output_file = tmp_path / "graph_weighted_undirected.toml"
        with graph_weighted(label="test_wgraph", directed=False, output=str(output_file)) as g:
            g.add_node(0, "A")
            g.add_node(1, "B")
            g.add_edge(0, 1, 3.5)

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"
        assert 'kind = "graph_weighted"' in content
        assert "weight = 3.5" in content

    def test_graph_weighted_directed(self, tmp_path: Path) -> None:
        r"""测试有向带权图"""
        output_file = tmp_path / "graph_weighted_directed.toml"
        with graph_weighted(directed=True, output=str(output_file)) as g:
            g.add_node(0, "A")
            g.add_node(1, "B")
            g.add_edge(0, 1, 2.0)
            g.add_edge(1, 0, 3.0)

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_graph_weighted_update_weight(self, tmp_path: Path) -> None:
        r"""测试更新边权重"""
        output_file = tmp_path / "graph_weighted_update.toml"
        with graph_weighted(output=str(output_file)) as g:
            g.add_node(0, "A")
            g.add_node(1, "B")
            g.add_edge(0, 1, 1.0)
            g.update_weight(0, 1, 5.0)

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_graph_update_node_label(self, tmp_path: Path) -> None:
        r"""测试更新节点标签"""
        output_file = tmp_path / "graph_update_label.toml"
        with graph_weighted(output=str(output_file)) as g:
            g.add_node(0, "A")
            g.update_node_label(0, "X")

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_graph_weighted_negative_weight(self, tmp_path: Path) -> None:
        r"""测试负权重"""
        output_file = tmp_path / "graph_negative_weight.toml"
        with graph_weighted(output=str(output_file)) as g:
            g.add_node(0, "A")
            g.add_node(1, "B")
            g.add_edge(0, 1, -5.5)

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_graph_weighted_zero_weight(self, tmp_path: Path) -> None:
        r"""测试零权重"""
        output_file = tmp_path / "graph_zero_weight.toml"
        with graph_weighted(output=str(output_file)) as g:
            g.add_node(0, "A")
            g.add_node(1, "B")
            g.add_edge(0, 1, 0.0)

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_graph_weighted_update_nonexistent_edge_error(self, tmp_path: Path) -> None:
        r"""测试更新不存在的边权重错误"""
        output_file = tmp_path / "graph_update_nonexist_edge.toml"
        with pytest.raises(StructureError):
            with graph_weighted(output=str(output_file)) as g:
                g.add_node(0, "A")
                g.add_node(1, "B")
                g.update_weight(0, 1, 5.0)

        content: str = output_file.read_text(encoding="utf-8")
        assert "[error]" in content

    def test_graph_weighted_remove_edge(self, tmp_path: Path) -> None:
        r"""测试删除带权边"""
        output_file = tmp_path / "graph_weighted_remove_edge.toml"
        with graph_weighted(output=str(output_file)) as g:
            g.add_node(0, "A")
            g.add_node(1, "B")
            g.add_edge(0, 1, 1.0)
            g.remove_edge(0, 1)

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_graph_weighted_clear(self, tmp_path: Path) -> None:
        r"""测试带权图清空"""
        output_file = tmp_path / "graph_weighted_clear.toml"
        with graph_weighted(output=str(output_file)) as g:
            g.add_node(0, "A")
            g.add_node(1, "B")
            g.remove_node(0)
            g.remove_node(1)

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"


class TestGlobalConfigIntegration:
    r"""全局配置集成测试"""

    def test_config_remarks(self, tmp_path: Path) -> None:
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

        content: str = output_file.read_text(encoding="utf-8")
        assert "[remarks]" in content
        assert 'title = "Test Title"' in content
        assert 'author = "Test Author"' in content
        assert 'comment = "Test Comment"' in content

        config()

    def test_config_output_path(self, tmp_path: Path) -> None:
        r"""测试配置输出路径"""
        output_file = tmp_path / "custom_output.toml"
        config(output_path=str(output_file))

        with stack() as s:
            s.push(1)

        assert output_file.exists()
        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

        config()

    def test_config_only_title(self, tmp_path: Path) -> None:
        r"""测试只配置title"""
        output_file = tmp_path / "config_title_only.toml"
        config(output_path=str(output_file), title="Only Title")

        with stack() as s:
            s.push(1)

        content: str = output_file.read_text(encoding="utf-8")
        assert 'title = "Only Title"' in content

        config()

    def test_config_only_author(self, tmp_path: Path) -> None:
        r"""测试只配置author"""
        output_file = tmp_path / "config_author_only.toml"
        config(output_path=str(output_file), author="Only Author")

        with stack() as s:
            s.push(1)

        content: str = output_file.read_text(encoding="utf-8")
        assert 'author = "Only Author"' in content

        config()

    def test_config_only_comment(self, tmp_path: Path) -> None:
        r"""测试只配置comment"""
        output_file = tmp_path / "config_comment_only.toml"
        config(output_path=str(output_file), comment="Only Comment")

        with stack() as s:
            s.push(1)

        content: str = output_file.read_text(encoding="utf-8")
        assert 'comment = "Only Comment"' in content

        config()


class TestEdgeCases:
    r"""边缘情况测试"""

    def test_empty_structure(self, tmp_path: Path) -> None:
        r"""测试空数据结构"""
        output_file = tmp_path / "empty.toml"
        with stack(output=str(output_file)):
            ...

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

        state_count: int = content.count("[[states]]")
        assert state_count == 1

    def test_special_characters_in_string(self, tmp_path: Path) -> None:
        r"""测试字符串中的特殊字符"""
        output_file = tmp_path / "special_chars.toml"
        with stack(output=str(output_file)) as s:
            s.push('hello"world')
            s.push("line1\\nline2")

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_large_numbers(self, tmp_path: Path) -> None:
        r"""测试大数字"""
        output_file = tmp_path / "large_numbers.toml"
        with stack(output=str(output_file)) as s:
            s.push(10**18)
            s.push(-10**18)
            s.push(1.7976931348623157e308)

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_many_operations(self, tmp_path: Path) -> None:
        r"""测试大量操作"""
        output_file = tmp_path / "many_ops.toml"
        with stack(output=str(output_file)) as s:
            for i in range(100):
                s.push(i)
            for _ in range(50):
                s.pop()

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

        state_count: int = content.count("[[states]]")
        step_count: int = content.count("[[steps]]")
        assert state_count == 151
        assert step_count == 150

    def test_node_label_boundary(self, tmp_path: Path) -> None:
        r"""测试节点标签边界"""
        output_file = tmp_path / "label_boundary.toml"
        with graph_undirected(output=str(output_file)) as g:
            g.add_node(0, "A")
            g.add_node(1, "A" * 32)

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_node_label_too_long_error(self, tmp_path: Path) -> None:
        r"""测试节点标签过长错误"""
        output_file = tmp_path / "label_too_long.toml"
        with pytest.raises(StructureError):
            with graph_undirected(output=str(output_file)) as g:
                g.add_node(0, "A" * 33)

        content: str = output_file.read_text(encoding="utf-8")
        assert "[error]" in content

    def test_node_label_empty_error(self, tmp_path: Path) -> None:
        r"""测试节点标签为空错误"""
        output_file = tmp_path / "label_empty.toml"
        with pytest.raises(StructureError):
            with graph_undirected(output=str(output_file)) as g:
                g.add_node(0, "")

        content: str = output_file.read_text(encoding="utf-8")
        assert "[error]" in content

    def test_unicode_strings(self, tmp_path: Path) -> None:
        r"""测试Unicode字符串"""
        output_file = tmp_path / "unicode.toml"
        with stack(output=str(output_file)) as s:
            s.push("你好")
            s.push("世界")
            s.push("🌍")

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_very_long_label(self, tmp_path: Path) -> None:
        r"""测试最大长度标签"""
        output_file = tmp_path / "max_label.toml"
        with graph_undirected(output=str(output_file)) as g:
            max_label: str = "X" * 32
            g.add_node(0, max_label)

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_mixed_type_values(self, tmp_path: Path) -> None:
        r"""测试混合类型值"""
        output_file = tmp_path / "mixed_types.toml"
        with stack(output=str(output_file)) as s:
            s.push(1)
            s.push(2.5)
            s.push("text")
            s.push(True)
            s.push(False)
            s.pop()

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"


class TestComplexScenarios:
    r"""复杂场景测试"""

    def test_bst_complex_deletion(self, tmp_path: Path) -> None:
        r"""测试 BST 复杂删除场景"""
        output_file = tmp_path / "bst_complex.toml"
        with binary_search_tree(output=str(output_file)) as bst:
            for v in [50, 30, 70, 20, 40, 60, 80, 35, 45]:
                bst.insert(v)
            bst.delete(30)
            bst.delete(50)

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_graph_complex_operations(self, tmp_path: Path) -> None:
        r"""测试图的复杂操作"""
        output_file = tmp_path / "graph_complex.toml"
        with graph_weighted(directed=True, output=str(output_file)) as g:
            for i in range(5):
                g.add_node(i, chr(ord('A') + i))

            g.add_edge(0, 1, 1.0)
            g.add_edge(0, 2, 2.0)
            g.add_edge(1, 2, 1.5)
            g.add_edge(2, 3, 2.5)
            g.add_edge(3, 4, 1.0)
            g.add_edge(4, 0, 3.0)

            g.update_weight(0, 1, 10.0)
            g.remove_node(2)
            g.update_node_label(0, "Start")

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_linked_list_complex_operations(self, tmp_path: Path) -> None:
        r"""测试链表的复杂操作"""
        output_file = tmp_path / "list_complex.toml"
        with double_linked_list(output=str(output_file)) as dlist:
            nodes: List[int] = []
            for i in range(5):
                nodes.append(dlist.insert_tail(i))

            dlist.insert_after(nodes[2], 100)
            dlist.insert_before(nodes[2], 200)
            dlist.delete_head()
            dlist.delete_tail()
            dlist.reverse()

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_heap_sort_simulation(self, tmp_path: Path) -> None:
        r"""测试堆排序模拟"""
        output_file = tmp_path / "heap_sort.toml"
        with heap(heap_type="min", output=str(output_file)) as h:
            values: List[int] = [5, 2, 8, 1, 9, 3]
            for v in values:
                h.insert(v)
            for _ in range(len(values)):
                h.extract()

        content: str = output_file.read_text(encoding="utf-8")
        validator = TomlValidator(content)
        assert validator.validate_all(), f"Validation errors: {validator.errors}"

    def test_graph_cycle_detection_scenario(self, tmp_path: Path) -> None:
        r"""测试图环检测场景"""
        output_file = tmp_path / "graph_cycle.toml"
        with graph_directed(output=str(output_file)) as g:
            for i in range(4):
                g.add_node(i, f"Node{i}")

            g.add_edge(0, 1)
            g.add_edge(1, 2)
            g.add_edge(2, 3)
            g.add_edge(3, 1)

        content: str = output_file.read_text(encoding="utf-8")
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

    def test_factory_function_signatures(self) -> None:
        r"""测试工厂函数签名一致性"""
        functions_with_label: List[Any] = [
            stack, queue, single_linked_list, double_linked_list,
            binary_tree, binary_search_tree, graph_undirected, graph_directed
        ]
        for func in functions_with_label:
            sig: str = str(func.__code__.co_varnames)
            assert "label" in sig or "output" in sig

    def test_all_structures_context_manager(self, tmp_path: Path) -> None:
        r"""测试所有数据结构都是上下文管理器"""
        structures: List[Any] = [
            stack, queue, single_linked_list, double_linked_list,
            binary_tree, binary_search_tree, heap,
            graph_undirected, graph_directed, graph_weighted
        ]
        for struct_func in structures:
            output_file = tmp_path / f"{struct_func.__name__}_ctx.toml"
            with struct_func(output=str(output_file)):
                ...
            assert output_file.exists()
