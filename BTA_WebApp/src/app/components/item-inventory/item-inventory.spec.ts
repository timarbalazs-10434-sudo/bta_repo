import { ComponentFixture, TestBed } from '@angular/core/testing';

import { ItemInventory } from './item-inventory';

describe('ItemInventory', () => {
  let component: ItemInventory;
  let fixture: ComponentFixture<ItemInventory>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      imports: [ItemInventory]
    })
    .compileComponents();

    fixture = TestBed.createComponent(ItemInventory);
    component = fixture.componentInstance;
    await fixture.whenStable();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
