import { Component, Input } from '@angular/core';

@Component({
  selector: 'app-item-inventory',
  imports: [],
  templateUrl: './item-inventory.html',
  styleUrl: './item-inventory.css',
})
export class ItemInventory {
 @Input() sessionID :number = 0;
  clicktry(){
    console.log(this.sessionID)
  }
}
