import { Component, inject, signal } from '@angular/core';
import { InventoryService } from '../services/inventory-service';
import { UserModel } from '../model/user-model.type';
import { catchError } from 'rxjs';
import { InventoryModel } from '../model/inventory-model.type';

@Component({
  selector: 'app-inventory',
  imports: [],
  templateUrl: './inventory.html',
  styleUrl: './inventory.css',
})
export class Inventory{
  users = [
    { name: 'abcdefsadadasdasdasdsadasdada',LastUpdated: "2026.13.22", DaySpent: "10000" , InvLength: "2121200"},
    { name: 'Béla',LastUpdated: "2028.13.22", DaySpent: "0", InvLength: "2000"},
    { name: 'NagyonHosszúNévValaki',LastUpdated: "1999.13.22", DaySpent: "1900", InvLength: "200"}
  ];
  nameTruncate(name: string) {
  if (!name) return '';

  return name.length > 12
    ? name.slice(0, 13) + '...'
    : name;
  }

  
  currentId:number = 0;

  getID(ids: number){
    this.currentId = ids
    console.log(ids)
    return ids;
  }

  //Innentől jön a Service:

  userService = inject(InventoryService)
  userItems = signal<Array<UserModel>>([])
 
  loadUser():void{
    this.userService.getUser()
    .pipe(
    catchError((err) =>{
      console.log(err);
      throw err;
      })
    )
    .subscribe((users) => {
        console.log('UserInfo Válasz:', users);
        console.log('Válasz típusa:', typeof users);
        if (!users || users.length == 0) {
          console.log('Nincs userről adat!');
          return;
        }
        this.userService.setFullUser(users)
    });
  } 
  

  invService = inject(InventoryService)
  invItems = signal<Array<InventoryModel>>([]);

  loadInventory(): void {
    this.invService.getInventory()
    .pipe(
      catchError((err) =>{
      console.log(err);
      throw err;
      })
    )
    .subscribe((inventory) =>{
      //this.invItems.set(inventory);
      console.log(inventory)
    })
  }
}
