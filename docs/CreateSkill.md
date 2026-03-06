当指令为“创建一个 SKILL”时，请严格执行以下规则：

创建目录：在 .opencode/skills/ 目录下，新建一个与该技能相关的英文名文件夹。

创建文件：在该文件夹内，仅创建一个名为 SKILL.md 的文件（严格固定此命名）。

内容格式：必须采用标准格式（参考 .opencode/skills/ProjectLifecycleManager/SKILL.md）。

name：填写技能名称。

description（最高优先级）：这是 AI 检索和触发技能的核心依据。必须使用明确的条件句式，例如：“当用户提到 [某词语] 的时候，执行 [某操作]”。