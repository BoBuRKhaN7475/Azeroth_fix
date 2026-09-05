-- mod-progressive-wotlk
-- Database: world
-- Maqsad: Faza holati saqlanadigan jadval

-- Jadval yaratish (agar mavjud bo'lmasa)
CREATE TABLE IF NOT EXISTS `progressive_phase` (
    `current_phase`  TINYINT UNSIGNED NOT NULL DEFAULT 1
                     COMMENT 'Joriy faza (1-5)',
    `phase_name`     VARCHAR(128) NOT NULL DEFAULT ''
                     COMMENT 'Faza nomi (ma''lumot uchun)',
    `activated_at`   TIMESTAMP NULL DEFAULT NULL
                     COMMENT 'Faza qachon faollashtirildi',
    PRIMARY KEY (`current_phase`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4
  COMMENT='Progressive WotLK faza holati';

-- Boshlang''ich ma''lumot: Phase 1 dan boshlaymiz
-- INSERT IGNORE: agar jadvalda allaqachon yozuv bo''lsa, o''zgartirmaydi
INSERT IGNORE INTO `progressive_phase`
    (`current_phase`, `phase_name`, `activated_at`)
VALUES
    (1, 'Phase 1 — Naxxramas / Obsidian Sanctum / Eye of Eternity', NOW());
