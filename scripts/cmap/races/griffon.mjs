export const isPlayable = true;

export const faces = ["griffon"];

export function spriteSheet(model) {
  return {
    cloneOf: "griffon",
    base:    "griffon",
    overlay: "griffon-wings"
  };
}

export function onToggled(statistics, toggled) {
  let modifier = toggled ? 1 : -1;

  statistics.strength   += modifier;
  statistics.perception += modifier;
  statistics.endurance  += modifier;
  statistics.agility    += modifier;
}
