-- ============================================================================
--  像素农场 · 数据库结构（第 10 周选做）
--
--  跑法：
--      mysql -u root -p < sql/schema.sql
--  或者在 mysql 里：
--      SOURCE D:/ds/pixelfarm/sql/schema.sql;
--
--  这张表和你的 C 结构体是一一对应的，对照关系见文件末尾。
-- ============================================================================

DROP DATABASE IF EXISTS pixelfarm;
CREATE DATABASE pixelfarm
    DEFAULT CHARACTER SET utf8mb4
    COLLATE utf8mb4_unicode_ci;
USE pixelfarm;

-- ---------------------------------------------------------------- 作物表
-- 对应 farm.h 里的 Crop 结构体
CREATE TABLE crops (
    id          INT          PRIMARY KEY,
    name        VARCHAR(20)  NOT NULL UNIQUE,
    price       INT          NOT NULL,        -- 今日收购价（分/斤）
    grow_days   INT          NOT NULL,
    seed_cost   INT          NOT NULL,        -- 种子成本（分）
    demand      INT          NOT NULL DEFAULT 50,
    net_profit  INT          NOT NULL DEFAULT 0,
    INDEX idx_price (price),
    INDEX idx_days (grow_days)
) ENGINE=InnoDB;

-- ---------------------------------------------------------------- 地块表
-- 对应 farm.h 里的 Plot 结构体。36 行，和 6×6 一一对应
CREATE TABLE plots (
    x           INT          NOT NULL,
    y           INT          NOT NULL,
    crop_id     INT          NULL,            -- NULL 表示空地
    state       TINYINT      NOT NULL DEFAULT 0,   -- 0荒 1垦 2播 3长 4熟 5枯
    planted_day INT          NOT NULL DEFAULT 0,
    ripe_day    INT          NOT NULL DEFAULT 0,
    water       INT          NOT NULL DEFAULT 0,
    PRIMARY KEY (x, y),
    INDEX idx_state (state),
    CONSTRAINT fk_plot_crop FOREIGN KEY (crop_id)
        REFERENCES crops(id) ON DELETE SET NULL
) ENGINE=InnoDB;

-- ---------------------------------------------------------------- 仓库表
-- 对应 farm.h 里的 Item 结构体（第 1 周的顺序表）
CREATE TABLE barn (
    crop_id     INT          PRIMARY KEY,
    count       INT          NOT NULL DEFAULT 0,
    quality     TINYINT      NOT NULL DEFAULT 3,
    CONSTRAINT fk_barn_crop FOREIGN KEY (crop_id)
        REFERENCES crops(id) ON DELETE CASCADE
) ENGINE=InnoDB;

-- ---------------------------------------------------------------- 订单表
-- 对应 farm.h 里的 Order 结构体（第 3 周的队列）
CREATE TABLE orders (
    id          BIGINT       PRIMARY KEY AUTO_INCREMENT,
    crop_id     INT          NOT NULL,
    count       INT          NOT NULL,
    price       INT          NOT NULL,
    status      TINYINT      NOT NULL DEFAULT 0,   -- 0待 1送 2完 3取消
    day         INT          NOT NULL,
    created_at  DATETIME     NOT NULL DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_status (status),
    INDEX idx_day (day),
    CONSTRAINT fk_order_crop FOREIGN KEY (crop_id)
        REFERENCES crops(id)
) ENGINE=InnoDB;

-- ---------------------------------------------------------------- 操作日志表
-- 对应 growth.h 里的 LNode（第 2 周的单链表）
CREATE TABLE trace (
    id          BIGINT       PRIMARY KEY AUTO_INCREMENT,
    day         INT          NOT NULL,
    action      INT          NOT NULL,        -- 1垦 2播 3浇 4收 5卖
    detail      VARCHAR(64)  NOT NULL DEFAULT '',
    ts          DATETIME     NOT NULL DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_day (day)
) ENGINE=InnoDB;

-- ---------------------------------------------------------------- 初始数据：从 crops.csv 导
-- 如果你的 MySQL 不允许 LOAD DATA LOCAL，就用 Python 脚本导（见 tools/csv2mysql.py）
--
-- LOAD DATA LOCAL INFILE 'data/crops.csv'
-- INTO TABLE crops
-- FIELDS TERMINATED BY ',' LINES TERMINATED BY '\n' IGNORE 1 LINES
-- (id, name, grow_days, seed_cost, price, demand);

-- 先用几条顶上，保证表不是空的
INSERT INTO crops (id, name, price, grow_days, seed_cost, demand) VALUES
  (0,  '小麦',   240,  4, 100, 80),
  (1,  '玉米',   520,  7, 180, 60),
  (2,  '胡萝卜', 190,  3,  80, 90),
  (3,  '番茄',   470,  6, 160, 55),
  (4,  '南瓜',  1350, 12, 400, 30);

-- 36 块地，全部是荒地
INSERT INTO plots (x, y, crop_id, state, water)
SELECT x.n, y.n, NULL, 0, 20
FROM (SELECT 0 n UNION SELECT 1 UNION SELECT 2 UNION SELECT 3
      UNION SELECT 4 UNION SELECT 5) x,
     (SELECT 0 n UNION SELECT 1 UNION SELECT 2 UNION SELECT 3
      UNION SELECT 4 UNION SELECT 5) y;

-- ---------------------------------------------------------------- 验证
SELECT '建库完成' AS result,
       (SELECT COUNT(*) FROM crops) AS 作物数,
       (SELECT COUNT(*) FROM plots) AS 地块数;

-- ============================================================================
--  你的 C 结构体  ↔  数据库里的东西
--
--  Crop.name        ↔  crops.name
--  Crop.price       ↔  crops.price
--  Plot.state       ↔  plots.state（枚举值直接存成数字）
--  Plot.crop_id     ↔  plots.crop_id（外键，能保证不出现不存在的作物）
--  Item             ↔  barn 的一行
--  Order            ↔  orders 的一行
--  LNode            ↔  trace 的一行
--
--  你可以想一个问题：为什么 plots 的主键是 (x,y) 而不是自增 id？
--  提示：你打印一张 6×6 的地图时，是按 (x,y) 直接算下标快，还是先查 id 快？
-- ============================================================================
